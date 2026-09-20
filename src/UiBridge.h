#pragma once

#include <cstdint>
#include <string_view>

// UiBridge — the C++ -> JS push helpers.
//
// The PrismaUI.dll ABI, view creation and the JS callback registration live in
// PrismaUI.h (the single indirection point). This file only marshals the
// plugin's data pushes onto the game thread through the SKSE TaskInterface,
// matching the original's `GetTaskInterface()->AddTask(...)` pattern
// (FUN_180037e90 in the decompilation), and names the JS entry points the
// controller-0.6.1.html defines (slppState / slppSetCompatible /
// slppSetSearchQuery / slppCatalogReset / slppCatalogChunk / slppCatalogDone).
namespace UiBridge
{
	// window.slppState(<scene-state JSON object>)
	void PushState(std::string_view a_json);

	// window.slppSetCompatible(<JSON array of scene ids>)
	void PushCompatible(std::string_view a_json);

	// window.slppSetSearchQuery(<string>)
	void SetSearchQuery(std::string_view a_query);

	// slppCatalogReset(total) + slppCatalogChunk(rows) + slppCatalogProgress +
	// slppCatalogDone(total). `a_rowsJson` is a JSON array of
	// {"id","name","tags","package"} rows.
	void PushCatalog(std::string_view a_rowsJson, std::int32_t a_total);

	// The incremental push CatalogAppend() queues after each native call:
	// window.slppCatalogChunk(rows) followed by window.slppCatalogProgress.
	void PushCatalogChunk(std::string_view a_rowsJson, std::int32_t a_loaded, std::int32_t a_total);

	// True once RequestPluginAPI(1) succeeded and the view was created.
	bool IsAvailable();

	// Delegate to PrismaUI::InvokeJs (kept for the existing call sites).
	void InvokeJs(const char* a_functionName, std::string_view a_argument);
}  // namespace UiBridge