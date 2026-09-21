#include "Papyrus/CatalogNatives.h"

#include "PCH.h"
#include "Catalog.h"
#include "PrismaUI.h"
#include "UiBridge.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------
// The original compiles every native *body* in `src\main.cpp` inside the file's
// GLOBAL anonymous namespace, so each body's baked __FUNCSIG__ reads
//   `anonymous-namespace'::Papyrus_CatalogBegin(__cdecl)(RE::StaticFunctionTag*, ...)
// (verified with `strings artifacts/SexLabPrism.dll`). The `#[line N "src\\main.cpp"]`
// pins below set both the recorded filename and the line the source_loc bakes.
// ---------------------------------------------------------------------------

namespace
{
	constexpr std::string_view kClassName = "SexLabPrismNative"sv;

	// FUN_18002bee0: "..." with JSON escaping.
	std::string QuoteJson(std::string_view a_in)
	{
		std::string out;
		out.reserve(a_in.size() + 2);
		out += '"';
		for (const char c : a_in) {
			switch (c) {
			case '"':  out += "\\\""; break;
			case '\\': out += "\\\\"; break;
			case '\n': out += "\\n"; break;
			case '\r': out += "\\r"; break;
			case '\t': out += "\\t"; break;
			default:
				if (static_cast<unsigned char>(c) < 0x20) {
					char buf[8];
					std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
					out += buf;
				} else {
					out += c;
				}
			}
		}
		out += '"';
		return out;
	}

	// --- catalogue state (DAT_18009c1d8 / DAT_1800951f0) -------------------
	std::mutex                            g_catalogMutex;
	std::vector<Catalog::Record>          g_catalogRecords;
	std::unordered_map<std::string, std::size_t, Catalog::Fnv1aHash> g_catalogIndex;
	std::int32_t                          g_catalogExpectedTotal = 0;
	std::atomic<bool>                     g_catalogReady         = false;
	bool                                  g_catalogBuilding      = false;
	std::uint64_t                         g_catalogBuildStart    = 0;

	// Original: FUN_18001d7d0 (a QPC-family clock) stored at DAT_18009c238 and
	// diffed in nanoseconds (recon/NATIVES-RECOVERED.md §3.9).
	std::uint64_t NowTicks()
	{
		return static_cast<std::uint64_t>(
			std::chrono::steady_clock::now().time_since_epoch().count());
	}

	// FUN_1800273a0 — one catalogue row as JSON. The original emits the fields in
	// the order id, name, package, tags; package defaults to "Unsorted"; the tag
	// list is ONE comma-separated string split here, each tag quoted, joined by
	// ',' (verified from FUN_1800273a0's memchr loop and the literals at
	// 0x18005d0a8..0x18005d0dc: `,"tags":[`, `,"package":`, `,"name":`, `{"id":`,
	// `]}` and the "Unsorted" immediate 0x646574726f736e55).
	std::string FormatRow(const Catalog::Record& a_record)
	{
		std::string row;
		row += "{\"id\":";
		row += QuoteJson(a_record.id);
		row += ",\"name\":";
		row += QuoteJson(a_record.name);
		row += ",\"package\":";
		row += QuoteJson(a_record.package.empty() ? std::string_view{ "Unsorted" } : std::string_view{ a_record.package });
		row += ",\"tags\":[";
		const std::string_view tags = a_record.tags;
		std::size_t            start = 0;
		bool                   firstTag = true;
		while (start < tags.size()) {
			std::size_t comma = tags.find(',', start);
			if (comma == std::string_view::npos) {
				comma = tags.size();
			}
			if (comma > start) {
				if (!firstTag) {
					row += ',';
				}
				row += QuoteJson(tags.substr(start, comma - start));
				firstTag = false;
			}
			if (comma == tags.size()) {
				break;
			}
			start = comma + 1;
		}
		row += "]}";
		return row;
	}

	std::string RowsJson(const std::vector<Catalog::Record>& a_records)
	{
		std::string json;
		json.reserve(a_records.size() * 96 + 2);
		json += '[';
		for (std::size_t i = 0; i < a_records.size(); ++i) {
			if (i != 0) {
				json += ',';
			}
			json += FormatRow(a_records[i]);
		}
		json += ']';
		return json;
	}

	// =======================================================================
	// Natives — exact original signatures, global anonymous namespace.
	// =======================================================================

	// 0x18002a000 — clear/reserve the 128-byte-record vector, reset the id index.
	void Papyrus_CatalogBegin(RE::StaticFunctionTag*, std::int32_t a_total)
	{
		{
			std::lock_guard lock{ g_catalogMutex };
			g_catalogRecords.clear();
			g_catalogIndex.clear();
			if (a_total > 0) {
				g_catalogRecords.reserve(static_cast<std::size_t>(a_total));
			}
			g_catalogExpectedTotal = a_total;
			g_catalogBuildStart    = NowTicks();
		}
		g_catalogReady    = false;
		g_catalogBuilding = true;
#line 480 "src\\main.cpp"
		logger::info("Catalog build started: {} registered scenes expected", a_total);
#line 112
		UiBridge::InvokeJs("slppCatalogReset", std::to_string(a_total));
	}

	// 0x180029740 — append records, refresh the FNV-1a id -> index map.
	//
	// The original walks the three vectors in one locked pass, builds each row
	// through FUN_1800273a0 (one call per record), appends it to a single JSON
	// array that opens with `[` before the loop and closes with `]` after the
	// unlock, then dispatches it once through the SKSE task interface. The FNV-1a
	// hash lives inside the out-of-line map helper FUN_180020a60, not here.
	void Papyrus_CatalogAppend(
		RE::StaticFunctionTag*,
		std::vector<std::string> a_ids,
		std::vector<std::string> a_names,
		std::vector<std::string> a_tags)
	{
		std::string json = "[";
		bool        first = true;
		{
			std::lock_guard lock{ g_catalogMutex };
			for (std::size_t i = 0; i < a_ids.size(); ++i) {
				if (a_ids[i].empty()) {
					continue;
				}
				Catalog::Record record;
				record.id   = a_ids[i];
				record.name = (i < a_names.size() && !a_names[i].empty()) ? a_names[i] : a_ids[i];
				if (i < a_tags.size()) {
					record.tags = a_tags[i];
				}
				if (!first) {
					json += ',';
				}
				json += FormatRow(record);
				first = false;

				const std::size_t index = g_catalogRecords.size();
				auto              it    = g_catalogIndex.try_emplace(record.id).first;
				it->second = index;
				g_catalogRecords.push_back(record);
			}
		}
		json += ']';

		// The original queues one task that pushes the accumulated chunk and the
		// progress count (its `Papyrus_CatalogAppend::`2'::<lambda_1>`); the lambda
		// must be instantiated here so its `_Func_impl_no_alloc` wrapper is named
		// after this native and not after UiBridge.
		auto* const task = SKSE::GetTaskInterface();
		if (task) {
			const std::string rows{ json };
			const auto        loaded = std::to_string(static_cast<std::int32_t>(g_catalogRecords.size()));
			task->AddTask([rows, loaded]() {
				PrismaUI::InvokeJs("slppCatalogChunk", rows);
				PrismaUI::InvokeJs("slppCatalogProgress", loaded);
			});
		}
	}

	// 0x18002a410 — assign a package name to already-appended records.
	void Papyrus_CatalogPackage(RE::StaticFunctionTag*, std::string a_package, std::vector<std::string> a_ids)
	{
		std::lock_guard lock{ g_catalogMutex };
		std::size_t     mapped = 0;
		for (const auto& id : a_ids) {
			const auto it = g_catalogIndex.find(id);
			if (it != g_catalogIndex.end() && it->second < g_catalogRecords.size()) {
				g_catalogRecords[it->second].package = a_package;
				++mapped;
			}
		}
#line 533 "src\\main.cpp"
		logger::info("Catalog package '{}' mapped to {}/{} scenes", a_package, mapped, a_ids.size());
#line 161
	}

	// 0x18002a270 — ready/building flags + elapsed-ms log + UI push.
	void Papyrus_CatalogFinish(RE::StaticFunctionTag*)
	{
		std::size_t count = 0;
		{
			std::lock_guard lock{ g_catalogMutex };
			count = g_catalogRecords.size();
		}
		g_catalogReady    = true;
		g_catalogBuilding = false;
		const std::uint64_t ms = (NowTicks() - g_catalogBuildStart) / 1'000'000ull;
#line 547 "src\\main.cpp"
		logger::info("Catalog build finished: {} scenes in {} ms", count, ms);
#line 150
		UiBridge::InvokeJs("slppCatalogDone", std::to_string(count));
	}

	// 0x18002a8a0 — 9-byte leaf: movzx byte [ready].
	bool Papyrus_IsCatalogReady(RE::StaticFunctionTag*)
	{
		return g_catalogReady.load();
	}

	// 0x18002a830 — (end - begin) >> 7.
	std::int32_t Papyrus_GetCatalogCount(RE::StaticFunctionTag*)
	{
		std::lock_guard lock{ g_catalogMutex };
		return static_cast<std::int32_t>(g_catalogRecords.size());
	}

	// 0x18002a6e0 — hand the cached catalogue to the UI.
	void Papyrus_CatalogPublish(RE::StaticFunctionTag*)
	{
		std::vector<Catalog::Record> snapshot;
		{
			std::lock_guard lock{ g_catalogMutex };
			snapshot = g_catalogRecords;
		}
#line 577 "src\\main.cpp"
		logger::info("Catalog session cache reused: {} scenes already in UI", snapshot.size());
#line 177
		UiBridge::PushCatalog(RowsJson(snapshot), static_cast<std::int32_t>(snapshot.size()));
	}
}  // namespace

namespace Catalog
{
	std::size_t Fnv1aHash::operator()(std::string_view a_key) const noexcept
	{
		std::uint64_t hash = 0xcbf29ce484222325ull;
		for (const char c : a_key) {
			hash ^= static_cast<unsigned char>(c);
			hash *= 0x100000001b3ull;
		}
		return static_cast<std::size_t>(hash);
	}

	void RetryPublish(std::int32_t a_attempt)
	{
		std::vector<Record> snapshot;
		{
			std::lock_guard lock{ g_catalogMutex };
			snapshot = g_catalogRecords;
		}

		constexpr std::int32_t kMaxRetries = 5;
		if (a_attempt >= kMaxRetries) {
			logger::warn("Catalog still incomplete after {} retries", a_attempt);
			return;
		}

		const std::size_t chunkSize = (static_cast<std::size_t>(64) >> a_attempt) > 1
			? (static_cast<std::size_t>(64) >> a_attempt)
			: 1;
		logger::info("Catalog incomplete on UI side, republishing with smaller slices (attempt {})", a_attempt);

		const auto total = static_cast<std::int32_t>(snapshot.size());
		UiBridge::InvokeJs("slppCatalogReset", std::to_string(total));
		for (std::size_t i = 0; i < snapshot.size(); i += chunkSize) {
			const std::size_t remaining = snapshot.size() - i;
			const std::size_t end       = i + (chunkSize < remaining ? chunkSize : remaining);
			std::vector<Record> chunk(snapshot.begin() + static_cast<std::ptrdiff_t>(i),
			snapshot.begin() + static_cast<std::ptrdiff_t>(end));
			UiBridge::InvokeJs("slppCatalogChunk", RowsJson(chunk));
		}
		UiBridge::InvokeJs("slppCatalogDone", std::to_string(total));
	}
}  // namespace Catalog

namespace Papyrus::Natives
{
	void RegisterCatalog(VM* a_vm)
	{
		a_vm->RegisterFunction("CatalogBegin"sv, kClassName, Papyrus_CatalogBegin, true);
		a_vm->RegisterFunction("CatalogAppend"sv, kClassName, Papyrus_CatalogAppend, true);
		a_vm->RegisterFunction("CatalogPackage"sv, kClassName, Papyrus_CatalogPackage, true);
		a_vm->RegisterFunction("CatalogFinish"sv, kClassName, Papyrus_CatalogFinish, true);
		a_vm->RegisterFunction("IsCatalogReady"sv, kClassName, Papyrus_IsCatalogReady, true);
		a_vm->RegisterFunction("GetCatalogCount"sv, kClassName, Papyrus_GetCatalogCount, true);
		a_vm->RegisterFunction("CatalogPublish"sv, kClassName, Papyrus_CatalogPublish, true);
	}
}  // namespace Papyrus::Natives
