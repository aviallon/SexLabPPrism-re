#include "Catalog.h"

#include "PCH.h"
#include "Json.h"
#include "UiBridge.h"

#include <algorithm>
#include <atomic>
#include <chrono>

namespace Catalog
{
	namespace
	{
		std::mutex                            g_mutex;
		std::vector<Record>                   g_records;
		std::unordered_map<std::string, std::size_t, Fnv1aHash> g_index;
		std::int32_t                          g_expectedTotal = 0;
		std::atomic<bool>                     g_ready         = false;
		bool                                  g_building      = false;
		std::uint64_t                         g_buildStart    = 0;

		std::uint64_t NowTicks()
		{
			// Original: FUN_18001d7d0 (a QPC-family clock) stored at DAT_18009c238
			// and diffed in nanoseconds (recon/NATIVES-RECOVERED.md §3.9).
			return static_cast<std::uint64_t>(
				std::chrono::steady_clock::now().time_since_epoch().count());
		}

		// The Papyrus side hands tags as one joined string (papyrusUtil.StringJoin,
		// recon/PAPYRUS-CONTRACT.md §3). The UI consumes an array, so split here.
		// GUESS: the original's split delimiter was not recovered; accept ',', ';'
		// and '|' so the common separators all work.
		std::vector<std::string> SplitTags(std::string_view a_tags)
		{
			std::vector<std::string> out;
			std::string              current;
			for (const char c : a_tags) {
				if (c == ',' || c == ';' || c == '|') {
					if (!current.empty()) {
						out.push_back(std::move(current));
						current.clear();
					}
				} else {
					current += c;
				}
			}
			if (!current.empty()) {
				out.push_back(std::move(current));
			}
			return out;
		}

		std::string RowsJson(const std::vector<Record>& a_records)
		{
			std::string json;
			json.reserve(a_records.size() * 96 + 2);
			json += '[';
			bool first = true;
			for (const auto& record : a_records) {
				if (!first) {
					json += ',';
				}
				first = false;
				json += "{\"id\":";
				PrismJson::AppendQuoted(json, record.id);
				json += ",\"name\":";
				PrismJson::AppendQuoted(json, record.name);
				json += ",\"tags\":[";
				bool firstTag = true;
				for (const auto& tag : record.tags) {
					if (!firstTag) {
						json += ',';
					}
					firstTag = false;
					PrismJson::AppendQuoted(json, tag);
				}
				json += "],\"package\":";
				PrismJson::AppendQuoted(json, record.package);
				json += '}';
			}
			json += ']';
			return json;
		}
	}  // namespace

	std::size_t Fnv1aHash::operator()(std::string_view a_key) const noexcept
	{
		std::uint64_t hash = 0xcbf29ce484222325ull;
		for (const char c : a_key) {
			hash ^= static_cast<unsigned char>(c);
			hash *= 0x100000001b3ull;
		}
		return static_cast<std::size_t>(hash);
	}

	void Begin(RE::StaticFunctionTag*, std::int32_t a_total)
	{
		// The original (FUN_18002a000) releases DAT_1800951f0 *before* it touches
		// the ready/building flags; only the records/index mutation and the
		// build-start timestamp are inside the locked region.
		{
			std::lock_guard lock{ g_mutex };
			g_records.clear();
			g_index.clear();
			if (a_total > 0) {
				g_records.reserve(static_cast<std::size_t>(a_total));
			}
			g_expectedTotal = a_total;
			g_buildStart    = NowTicks();
		}
		g_ready         = false;
		g_building      = true;
#line 480
		logger::info("Catalog build started: {} registered scenes expected", a_total);
#line 112
		UiBridge::InvokeJs("slppCatalogReset", std::to_string(a_total));
	}

	void Append(RE::StaticFunctionTag*,
		const std::vector<std::string>& a_ids,
		const std::vector<std::string>&        a_names,
		const std::vector<std::string>&        a_tags)
	{
		std::lock_guard lock{ g_mutex };
		std::vector<Record> added;
		added.reserve(a_ids.size());
		for (std::size_t i = 0; i < a_ids.size(); ++i) {
			const auto& id = a_ids[i];
			if (id.empty()) {
				continue;
			}
			Record record;
			record.id   = id;
			// The original falls back to the id when the name slot is missing/empty.
			record.name = (i < a_names.size() && !a_names[i].empty()) ? a_names[i] : id;
			if (i < a_tags.size()) {
				record.tags = SplitTags(a_tags[i]);
			}
			const auto index = g_records.size();
			g_records.push_back(std::move(record));
			g_index[g_records.back().id] = index;
			added.push_back(g_records.back());
		}
		// The original has NO log call in Papyrus_CatalogAppend (the only log
		// immediates used by that body live in 0x180029200 OnMessage); an extra
		// logger::info here is a fabricated call that cannot byte-match.
		// The original queues a TaskInterface lambda here that incrementally
		// forwards the newly appended records and the progress to the view
		// (recon/NATIVES-RECOVERED.md §3.7).
		if (!added.empty()) {
			UiBridge::PushCatalogChunk(RowsJson(added),
				static_cast<std::int32_t>(g_records.size()), g_expectedTotal);
		}
	}

	void Package(RE::StaticFunctionTag*, const std::string& a_package, const std::vector<std::string>& a_ids)
	{
		std::lock_guard lock{ g_mutex };
		std::size_t     mapped = 0;
		for (const auto& id : a_ids) {
			const auto it = g_index.find(id);
			if (it != g_index.end() && it->second < g_records.size()) {
				g_records[it->second].package = a_package;
				++mapped;
			}
		}
#line 533
		logger::info("Catalog package '{}' mapped to {}/{} scenes", a_package, mapped, a_ids.size());
#line 161
	}

	void Finish(RE::StaticFunctionTag*)
	{
		std::size_t count = 0;
		{
			std::lock_guard lock{ g_mutex };
			count = g_records.size();
		}
		g_ready    = true;
		g_building = false;
		const std::uint64_t ms = (NowTicks() - g_buildStart) / 1'000'000ull;
		// Pinned to the original's source_loc line (immediate 0x223) recovered
		// from FUN_18002a270 at recon/decompiled/0x18002a270_Papyrus_CatalogFinish.c.
#line 547
		logger::info("Catalog build finished: {} scenes in {} ms", count, ms);
#line 150
		UiBridge::InvokeJs("slppCatalogDone", std::to_string(count));
	}

	bool IsReady(RE::StaticFunctionTag*)
	{
		// 0x18002a8a0 `IsCatalogReady` is a lock-free byte read (`movzx eax,[ready]`):
		// the original's flag is a standalone byte written with `xchg` by CatalogFinish,
		// not a field protected by the catalogue mutex.
		return g_ready.load();
	}

	std::int32_t Count(RE::StaticFunctionTag*)
	{
		std::lock_guard lock{ g_mutex };
		return static_cast<std::int32_t>(g_records.size());
	}

	void Publish(RE::StaticFunctionTag*)
	{
		std::vector<Record> snapshot;
		{
			std::lock_guard lock{ g_mutex };
			snapshot = g_records;
		}
		// Pinned to the original's source_loc line (immediate 0x241) recovered
		// from FUN_18002a6e0 at recon/decompiled/0x18002a6e0_Papyrus_CatalogPublish.c.
#line 577
		logger::info("Catalog session cache reused: {} scenes already in UI", snapshot.size());
#line 177
		UiBridge::PushCatalog(RowsJson(snapshot), static_cast<std::int32_t>(snapshot.size()));
	}

	void RetryPublish(std::int32_t a_attempt)
	{
		std::vector<Record> snapshot;
		{
			std::lock_guard lock{ g_mutex };
			snapshot = g_records;
		}

		// Retry budget: 5 smaller-slice attempts, then give up loudly.
		constexpr std::int32_t kMaxRetries = 5;
		if (a_attempt >= kMaxRetries) {
			logger::warn("Catalog still incomplete after {} retries", a_attempt);
			return;
		}

		// Halve the slice every attempt; GUESS: the original's exact byte budget
		// was not recovered beyond the fact it shrank per attempt.
		// (`(std::min)` is parenthesised because Windows' minwindef.h defines a
		// `min` macro that the MSVC/CLNG headers pull in.)
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