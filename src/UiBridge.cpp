#include "UiBridge.h"

#include "PCH.h"
#include "Presentation.h"
#include "PrismaUI.h"
#include "SceneState.h"

#include <format>

#include <cstdint>
#include <functional>
#include <string>

// The compatible-list JSON (DAT_18009c1a0) lives in the Papyrus natives TU with
// the other published JSON; its cross-TU accessor is declared here so this TU
// can re-read it inside PushCompatible's task lambda without owning the storage.
namespace SceneState
{
	std::string CurrentCompatibleJson();
}

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

	// The compatible-list body (the `PushCompatible(void)::<lambda_1>` twin).
	__declspec(noinline)
	void PushCompatibleBody(std::string a_json)
	{
		PrismaUI::InvokeJs("slppSetCompatible", a_json);
	}

	// Queues a UI update on the game thread. Original shape:
	// `anonymous-namespace'::QueueOnGameThread<T>` — file scope, not nested in
	// UiBridge, so the unity TU keeps a single global unnamed namespace.
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

	// -----------------------------------------------------------------------
	// 0x18002d170 JsCatalogReset / 0x18002d340 JsCatalogDone (the Class-1
	// "declared bodies in no force table" residue). In the original these two
	// are reachable only through the catalogue publish path; our reconstruction
	// of that path (Catalog::RetryPublish / CatalogNatives) calls
	// UiBridge::InvokeJs directly, so nothing references these bodies and
	// /Gy + /OPT:REF strips them before any symbol exists for the linker map to
	// bind. Force-link them with the mandated whole-table XOR sink below.
	// Matching-decomp artefact, NOT behaviour: no live runtime path calls them.
	const void* const kForceUnfoldUiBridge[] = {
		reinterpret_cast<const void*>(&JsCatalogReset),
		reinterpret_cast<const void*>(&JsCatalogDone),
	};
}  // namespace

extern "C" void ForceLink_UnfoldUiBridge()
{
	// Observe every element: seeing only element 0 lets the optimiser fold the
	// rest of the table away and the linker then strips those bodies.
	volatile std::uintptr_t sink = 0;
	for (const void* p : kForceUnfoldUiBridge) {
		sink ^= reinterpret_cast<std::uintptr_t>(p);
	}
	(void)sink;
}

// Keep the force-link entry point itself alive. /OPT:REF strips it (and, with
// it, the table's address-taken callees) unless something references it;
// src/main.cpp is single-owned, so instead of a call there we ask the linker
// directly. MSVC and clang-cl both emit this as a /INCLUDE directive, which
// lld-link honours, so zero hook lines are needed in src/main.cpp.
#ifdef _MSC_VER
#	pragma comment(linker, "/include:ForceLink_UnfoldUiBridge")
#endif

namespace UiBridge
{
	bool IsAvailable()
	{
		return PrismaUI::IsAvailable();
	}

	void InvokeJs(const char* a_functionName, std::string a_argument)
	{
		PrismaUI::InvokeJs(a_functionName, a_argument);
	}

	// The original `PublishSceneState` queues `PushState(void)::lambda_1` with NO
	// captures: the lambda re-reads the state-JSON global when the task runs.
	void PushState()
	{
		QueueOnGameThread([]() { PushStateBody(SceneState::CurrentStateJson()); });
	}

	// Same shape: `PushCompatible(void)::lambda_1`, no captures, global re-read.
	void PushCompatible()
	{
		QueueOnGameThread([]() { PushCompatibleBody(SceneState::CurrentCompatibleJson()); });
	}

	void PushCatalog(std::string a_rowsJson, std::int32_t a_total)
	{
		QueueOnGameThread([rows = std::move(a_rowsJson), a_total]() {
			JsCatalogReset(a_total);
			PrismaUI::InvokeJs("slppCatalogChunk", rows);
			JsCatalogDone(a_total);
		});
	}

	void PushCatalogChunk(std::string a_rowsJson, std::int32_t a_loaded, std::int32_t a_total)
	{
		const auto loaded = std::to_string(a_loaded);
		(void)a_total;  // slppCatalogProgress(loaded, total) takes two JS args;
		// the recovered InteropCall is single-argument (see PrismaUI.h), so only
		// `loaded` is sent and the JS `total || catalogTotal` fallback keeps the
		// total from slppCatalogReset. Marked: two-arg interop was not recovered.
		QueueOnGameThread([rows = std::move(a_rowsJson), loaded]() {
			PrismaUI::InvokeJs("slppCatalogChunk", rows);
			PrismaUI::InvokeJs("slppCatalogProgress", loaded);
		});
	}
}  // namespace UiBridge
