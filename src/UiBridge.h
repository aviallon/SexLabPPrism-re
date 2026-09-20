#pragma once

#include <cstdint>
#include <string_view>

// PrismaUI bridge.
//
// The original plugin does NOT talk to the browser itself: at message type 8
// (recon/NATIVES-RECOVERED.md §4.1) it GetProcAddress()es "RequestPluginAPI"
// from PrismaUI.dll, requests API version 1, creates the view
// "SexLabPPrism/controller-0.6.1.html" and then invokes JS entry points on it
// (window.slppState / slppSetCompatible / slppSetSearchQuery /
// slppCatalogReset / slppCatalogChunk / slppCatalogProgress / slppCatalogDone).
//
// The exact IVPrismaUI1 vtable layout is external to the original DLL and could
// not be recovered (recon/NATIVES-RECOVERED.md §6). Every interaction with the
// interface therefore funnels through ONE indirection point,
// UiBridge::InvokeJs() (UiBridge.cpp). If the vtable offsets ever need fixing,
// that is the only place to touch.
//
// Every push is marshalled onto the game thread through the SKSE
// TaskInterface, matching the original's `GetTaskInterface()->AddTask(...)`
// pattern (FUN_180037e90 in the decompilation).
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

	// True once RequestPluginAPI(1) succeeded and the view was created. Today the
	// view creation lives in the (not yet reconstructed) OnMessage handler, so
	// this is normally false and the pushes are dropped with one log line.
	bool IsAvailable();

	// The single PrismaUI indirection point. Declared publicly so the future
	// OnMessage view-wiring can reuse the same call site.
	void InvokeJs(const char* a_functionName, std::string_view a_argument);
}  // namespace UiBridge