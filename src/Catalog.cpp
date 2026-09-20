#include "Catalog.h"

#include "PCH.h"
#include "Json.h"
#include "UiBridge.h"

#include <algorithm>
#include <chrono>

namespace Catalog
{
	namespace
	{
		std::mutex                            g_mutex;
		std::vector<Record>                   g_records;
		std::unordered_map<std::string, std::size_t, Fnv1aHash> g_index;
		std::int32_t                          g_expectedTotal = 0;
		bool                                  g_ready         = false;
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

	void Begin(std::int32_t a_total)
	{
		std::lock_guard lock{ g_mutex };
		g_records.clear();
		g_index.clear();
		if (a_total > 0) {
			g_records.reserve(static_cast<std::size_t>(a_total));
		}
		g_expectedTotal = a_total;
		g_buildStart    = NowTicks();
		g_ready         = false;
		g_building      = true;
		logger::info("Catalog build started: {} registered scenes expected", a_total);
		UiBridge::InvokeJs("slppCatalogReset", std::to_string(a_total));
	}

	void Append(const std::vector<std::string>& a_ids,
		const std::vector<std::string>&        a_names,
		const std::vector<std::string>&        a_tags)
	{
		std::lock_guard lock{ g_mutex };
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
		}
		logger::info("Catalog append: {} scenes", g_records.size());
	}

	void Package(const std::string& a_package, const std::vector<std::string>& a_ids)
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
		logger::info("Catalog package '{}' mapped to {}/{} scenes", a_package, mapped, a_ids.size());
	}

	void Finish()
	{
		std::size_t count = 0;
		{
			std::lock_guard lock{ g_mutex };
			count = g_records.size();
		}
		g_ready    = true;
		g_building = false;
		const std::uint64_t ms = (NowTicks() - g_buildStart) / 1'000'000ull;
		logger::info("Catalog build finished: {} scenes in {} ms", count, ms);
		UiBridge::InvokeJs("slppCatalogDone", std::to_string(count));
	}

	bool IsReady()
	{
		std::lock_guard lock{ g_mutex };
		return g_ready;
	}

	std::int32_t Count()
	{
		std::lock_guard lock{ g_mutex };
		return static_cast<std::int32_t>(g_records.size());
	}

	void Publish()
	{
		std::vector<Record> snapshot;
		{
			std::lock_guard lock{ g_mutex };
			snapshot = g_records;
		}
		logger::info("Catalog session cache reused: {} scenes already in UI", snapshot.size());
		UiBridge::PushCatalog(RowsJson(snapshot), static_cast<std::int32_t>(snapshot.size()));
	}
}  // namespace Catalog