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
// The IVPrismaUI1 vtable is EXTERNAL to the original DLL, so only the slots the
// original actually calls are recovered, and only by BYTE OFFSET:
//   0x00 CreateView          0x18 RegisterCallback   0x40 DOM-ready / show
//   0x70 view flags/size     0xa8 console-message handler
// (recon/NATIVES-RECOVERED.md §6). The C++ -> JS invoke slot (InteropCall) was
// NOT traced; see kInteropCallVtblOffset below. Every dereference of the
// external interface funnels through this file (PrismaUI::Slot<>), which is the
// ONE indirection point the reconstruction is allowed to touch.
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
	// The single C++ -> JS indirection point. `a_functionName` is a page-level
	// JS function name (e.g. "slppState"), NOT a full script. The original
	// reached JS through PrismaUI's interop slot; that slot offset is unknown
	// (§6) and kLayoutConfirmed is false, so the call is logged and skipped
	// rather than mis-called. Change kInteropCallVtblOffset once it is known.
	void InvokeJs(const char* a_functionName, std::string_view a_argument);

	// --- focus helpers (used by FocusRecovery / Presentation) ------------------
	// Both are interop operations on the view; the exact slots were not
	// recovered, so they go through the same single-indirection gate.
	bool IsFocused();
	bool Unfocus();
}  // namespace PrismaUI