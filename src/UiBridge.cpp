#include "UiBridge.h"

#include "PCH.h"
#include "PrismaUI.h"

#include <format>

#include <cstdint>
#include <functional>
#include <string>

// The original's src\main.cpp compiled these two script bodies at global
// anonymous-namespace scope (mangled `anonymous-namespace'::JsCatalogReset` /
// `::JsCatalogDone`), which is why they read as `anonymous-namespace'` and not
// `UiBridge::anonymous-namespace'`. Keep the same scope here so the recovered
// names are reproducible. The formatter is std::format: the parity build uses
// spdlog's std_format=true config and the original DLL contains no fmt symbols
// (xmake.lua:32), so its `fmt::format` is the spdlog `fmt_lib` alias for std.
namespace
{
	// 0x18002d170 (94 insns): the catalog-open dispatch. The original formats
	//   "window.slppCatalogReset({});window.slppCatalogProgress(0,{});"
	// with the same `total` in both `{}` placeholders (format arg array = two int
	// args; both slots read the lambda's 32-bit capture at [this+8]) and hands
	// the result to InvokeOn (0x180028190), which guards on view + connected +
	// slot 0x60 before executing it via slot 0x08.
	void JsCatalogReset(std::int32_t a_total)
	{
		PrismaUI::InvokeOn(std::format(
			"window.slppCatalogReset({});window.slppCatalogProgress(0,{});", a_total, a_total));
	}

	// 0x18002d340 (67 insns): the catalog completion sibling.
	//   "window.slppCatalogDone({});" — one placeholder, arg read as a 64-bit
	//   capture at [this+8], again through InvokeOn.
	void JsCatalogDone(std::int64_t a_total)
	{
		PrismaUI::InvokeOn(std::format("window.slppCatalogDone({});", a_total));
	}
}  // namespace

namespace UiBridge
{
	namespace
	{
		template <class F>
		void QueueOnGameThread(F&& a_callable)
		{
			auto* const task = SKSE::GetTaskInterface();
			if (!task) {
				logger::warn("SKSE TaskInterface unavailable; dropping Prism UI update");
				return;
			}
			task->AddTask(std::function<void()>(std::forward<F>(a_callable)));
		}
	}  // namespace

	bool IsAvailable()
	{
		return PrismaUI::IsAvailable();
	}

	void InvokeJs(const char* a_functionName, std::string_view a_argument)
	{
		PrismaUI::InvokeJs(a_functionName, a_argument);
	}

	void PushState(std::string_view a_json)
	{
		const std::string json{ a_json };
		QueueOnGameThread([json]() { PrismaUI::InvokeJs("slppState", json); });
	}

	void PushCompatible(std::string_view a_json)
	{
		const std::string json{ a_json };
		QueueOnGameThread([json]() { PrismaUI::InvokeJs("slppSetCompatible", json); });
	}

	void SetSearchQuery(std::string_view a_query)
	{
		const std::string query{ a_query };
		QueueOnGameThread([query]() { PrismaUI::InvokeJs("slppSetSearchQuery", query); });
	}

	void PushCatalog(std::string_view a_rowsJson, std::int32_t a_total)
	{
		const std::string rows{ a_rowsJson };
		QueueOnGameThread([rows, a_total]() {
			JsCatalogReset(a_total);
			PrismaUI::InvokeJs("slppCatalogChunk", rows);
			JsCatalogDone(a_total);
		});
	}

	void PushCatalogChunk(std::string_view a_rowsJson, std::int32_t a_loaded, std::int32_t a_total)
	{
		const std::string rows{ a_rowsJson };
		const auto        loaded = std::to_string(a_loaded);
		(void)a_total;  // slppCatalogProgress(loaded, total) takes two JS args;
		// the recovered InteropCall is single-argument (see PrismaUI.h), so only
		// `loaded` is sent and the JS `total || catalogTotal` fallback keeps the
		// total from slppCatalogReset. Marked: two-arg interop was not recovered.
		QueueOnGameThread([rows, loaded]() {
			PrismaUI::InvokeJs("slppCatalogChunk", rows);
			PrismaUI::InvokeJs("slppCatalogProgress", loaded);
		});
	}
}  // namespace UiBridge