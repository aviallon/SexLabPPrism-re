#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// The animation catalogue, shared by the Catalog* Papyrus natives.
//
// Recovered layout (recon/NATIVES-RECOVERED.md §0/§3.6–3.12): a vector of
// 0x80-byte records, an id -> record-index unordered_map, a `ready` flag, a
// `building` flag and a build-start tick. CatalogBegin clears+reserves the
// vector, resets the map and stamps the start tick; CatalogFinish flips the
// flags and reports the elapsed ms.
namespace Catalog
{
	// FNV-1a 64-bit, exactly the constants the original uses for its id index
	// (0xcbf29ce484222325 / 0x100000001b3, recon/NATIVES-RECOVERED.md §3.7).
	struct Fnv1aHash
	{
		std::size_t operator()(std::string_view a_key) const noexcept;
	};

	struct Record
	{
		std::string              id;               // +0x00
		std::string              name;             // +0x20
		std::vector<std::string> tags;             // +0x40
		std::uint64_t            reserved = 0;     // +0x58 (unidentified; padding to +0x60)
		std::string              package;          // +0x60
	};
	static_assert(sizeof(Record) == 0x80, "catalogue record stride must be 128 bytes");

	void         Begin(std::int32_t a_total);
	void         Append(const std::vector<std::string>& a_ids,
			 const std::vector<std::string>& a_names,
			 const std::vector<std::string>& a_tags);
	void         Package(const std::string& a_package, const std::vector<std::string>& a_ids);
	void         Finish();
	bool         IsReady();
	std::int32_t Count();
	void         Publish();

	// slppCatalogRetry: the UI received fewer rows than expected, so republish
	// with a smaller per-invoke slice. Logs "Catalog incomplete on UI side,
	// republishing with smaller slices (attempt {})" and, after the retry
	// budget is exhausted, "Catalog still incomplete after {} retries"
	// (recon/strings.txt §5).
	void         RetryPublish(std::int32_t a_attempt);
}  // namespace Catalog