#pragma once

#include <cstdint>
#include <string>

// UiBridge — the C++ -> JS push helpers.
//
// The PrismaUI.dll ABI, view creation and the JS callback registration live in
// PrismaUI.h (the single indirection point). This file only marshals the
// plugin's data pushes onto the game thread through the SKSE TaskInterface,
// matching the original's `GetTaskInterface()->AddTask(...)` pattern
// (FUN_180037e90 in the decompilation), and names the JS entry points the
// controller-0.6.1.html defines (slppState / slppSetCompatible /
// slppCatalogReset / slppCatalogChunk / slppCatalogDone).
//
// IMPORTANT SHAPES (recon/libh3-shapes.md §2, round 8):
//   PushState(void)        — NO argument; reads the published scene-state JSON
//                            global (DAT_18009c190) and captures NOTHING (the
//                            lambda re-reads the global inside the task).
//   PushCompatible(void)   — NO argument; same for the compatible-list JSON.
//   PushCatalog/PushCatalogChunk take by-value `std::string` (the original's
//   Papyrus_* natives use std::string, never string_view; the 16-byte
//   string_view -> std::string difference is the big natives' frame delta).
namespace UiBridge
{
	// window.slppState(<scene-state JSON>), queued on the game thread. Reads
	// SceneState::CurrentStateJson() inside the task lambda.
	void PushState();

	// window.slppSetCompatible(<JSON array of scene ids>), queued on the game
	// thread. Reads SceneState::CurrentCompatibleJson() inside the task lambda.
	void PushCompatible();

	// slppCatalogReset(total) + slppCatalogChunk(rows) + slppCatalogDone(total).
	// `a_rowsJson` is a JSON array of {"id","name","tags","package"} rows.
	void PushCatalog(std::string a_rowsJson, std::int32_t a_total);

	// The incremental push CatalogAppend() queues after each native call:
	// window.slppCatalogChunk(rows) followed by window.slppCatalogProgress.
	void PushCatalogChunk(std::string a_rowsJson, std::int32_t a_loaded, std::int32_t a_total);

	// True once RequestPluginAPI(1) succeeded and the view was created.
	bool IsAvailable();

	// Delegate to PrismaUI::InvokeJs (kept for the existing call sites).
	void InvokeJs(const char* a_functionName, std::string a_argument);
}  // namespace UiBridge
