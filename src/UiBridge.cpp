#include "UiBridge.h"

#include "PCH.h"
#include "Presentation.h"
#include "PrismaUI.h"

#include <format>

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

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
	__declspec(noinline)
	void JsCatalogReset(std::int32_t a_total)
	{
		PrismaUI::InvokeOn(std::format(
			"window.slppCatalogReset({});window.slppCatalogProgress(0,{});", a_total, a_total));
	}

	// 0x18002d340 (67 insns): the catalog completion sibling.
	//   "window.slppCatalogDone({});" — one placeholder, arg read as a 64-bit
	//   capture at [this+8], again through InvokeOn.
	// noinline: it has exactly one caller (the PublishCatalogToUi body below),
	// so LTCG would otherwise fold the 67-instruction body into that caller and
	// the linker would drop the standalone function the matcher looks for.
	__declspec(noinline)
	void JsCatalogDone(std::int64_t a_total)
	{
		PrismaUI::InvokeOn(std::format("window.slppCatalogDone({});", a_total));
	}

	// JSON-string escape for a JS argument (the original pre-quotes the modal
	// search text through the shared helper before pasting it into the script).
	std::string QuoteForJs(std::string_view a_text)
	{
		std::string out;
		out.reserve(a_text.size() + 2);
		out += '"';
		for (const char c : a_text) {
			if (c == '"' || c == '\\') {
				out += '\\';
			}
			out += c;
		}
		out += '"';
		return out;
	}

	// 0x180024450 (152 insns), the outlined body of `PushState`'s sibling lambda:
	// the queued `Papyrus_SetSearchQuery` task. The original builds
	//   window.slppSetSearchQuery(<quoted query>);
	// and hands it to InvokeOn, then re-applies the presentation. Without this
	// out-of-line body the only caller is the task lambda and LTCG inlines it.
	__declspec(noinline)
	void PushSearchQueryBody(std::string a_query)
	{
		PrismaUI::InvokeOn(std::format("window.slppSetSearchQuery({});", QuoteForJs(a_query)));
		Presentation::ApplyPresentation();
	}

	// 0x1800250f0 (302 insns): the outlined body of `PushState(void)::<lambda_1>`,
	// the task Papyrus_PublishSceneState queues. It re-derives the presentation,
	// then publishes the SAME current scene-state JSON to BOTH window.slppState
	// and window.slppVitals (the vitals panel consumes the identical payload).
	// The original reads the global JSON (DAT_18009c190); our copy carries it.
	__declspec(noinline)
	void PushStateBody(std::string a_json)
	{
		Presentation::ApplyPresentation();
		PrismaUI::InvokeOn(std::format("window.slppState({});", a_json));
		PrismaUI::InvokeOn(std::format("window.slppVitals({});", a_json));
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
		QueueOnGameThread([json]() { PushStateBody(json); });
	}

	void PushCompatible(std::string_view a_json)
	{
		const std::string json{ a_json };
		QueueOnGameThread([json]() { PrismaUI::InvokeJs("slppSetCompatible", json); });
	}

	void SetSearchQuery(std::string_view a_query)
	{
		const std::string query{ a_query };
		QueueOnGameThread([query]() { PushSearchQueryBody(query); });
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