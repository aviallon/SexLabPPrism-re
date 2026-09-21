#pragma once

#include <cstdint>
#include <string_view>

// PrismaUI bridge — the external-facing half of the plugin's messaging path.
//
// The original DLL never talks to a browser itself: OnMessage (0x180029200)
// loads PrismaUI.dll by NAME, GetProcAddress("RequestPluginAPI"), requests API
// version 1, creates the view "SexLabPPrism/controller-0.6.1.html" and wires
// six JS callbacks (recon/NATIVES-RECOVERED.md §4.1, recon/decompiled/
// 0x180029200_OnMessage.c). This translation unit reproduces that structure.
//
// The IVPrismaUI1 vtable is EXTERNAL to the original DLL. The offsets the
// original itself calls (0x00 CreateView, 0x18 RegisterCallback, 0x40 apply,
// 0x70 view config, 0xa8 console-message handler) are CONFIRMED from its
// OnMessage; the working consumer PrismaUITeleportMenu.dll additionally
// reveals the JS-invocation slots 0x08 (eval) and 0x10 (named function) and
// the focus slots 0x28/0x38. The complete recovered layout, with per-slot
// evidence, is in src/PrismaUI_vtbl.h. Every dereference of the external
// interface funnels through PrismaUI::Slot<>, the ONE indirection point.
namespace PrismaUI
{
	// The callback shape handed to RegisterCallback. The original's recovered
	// signatures are `SendAction(const char*)`, `RequestSearchInput(const char*)`
	// and `SetCollapsed(bool,const char*)` where the bool/`source` are the
	// OUTER function, not the ABI: the registered JS trampoline receives the JS
	// argument as a single C string. That is the shape used here.
	using JsCallback = void (*)(const char*);

	// OnConsoleMessage(uint64,enum PRISMA_UI_API::ConsoleMessageLevel,const char*)
	using ConsoleMessageCallback = void (*)(std::uint64_t, int, const char*);

	// --- lifecycle -------------------------------------------------------------

	// Reproduces `CreateViews()` / the body of OnMessage: load PrismaUI.dll,
	// RequestPluginAPI(1), CreateView("SexLabPPrism/controller-0.6.1.html"),
	// set view flags, install the console-message handler, register the six JS
	// callbacks, show the view. Safe to call once; a second call is a no-op.
	// Returns true once the view exists.
	bool CreateViews();

	// True once RequestPluginAPI(1) succeeded and the view was created.
	bool IsAvailable();

	// The raw interface / view handles, needed by FocusRecovery and
	// Presentation. Null until CreateViews() succeeds.
	void* Interface();
	void* View();

	// --- C++ -> JS -------------------------------------------------------------
	// Two recovered invocation slots (see PrismaUI_vtbl.h for the evidence):
	//   InvokeJs  -> slot 0x10, calls a named global JS function with a string
	//   ExecuteJs -> slot 0x08, evaluates a JS expression string
	// Both are now backed by real call sites recovered from
	// PrismaUITeleportMenu.dll, so the 12 JS-bridge functions have genuine
	// instruction streams instead of the retired 0x88 guess.
	void InvokeJs(const char* a_functionName, std::string_view a_argument);
	void ExecuteJs(const char* a_code);

	// --- focus helpers (used by FocusRecovery / Presentation) ------------------
	// Slot 0x38 (query) and slot 0x28 (set interactive). The query's return
	// convention is not recovered; Unfocus reports whether the slot was called.
	bool IsFocused();
	bool Unfocus();
}  // namespace PrismaUI