#pragma once

// ============================================================================
// IVPrismaUI1 vtable — RECOVERED FROM THE DLLs ON DISK
// ============================================================================
//
// The PrismaUI runtime is external to SexLabPPrism.dll; the plugin reaches it
// only by name:
//
//     HMODULE h = GetModuleHandleA("PrismaUI.dll");               // 0x180029220
//     auto request = GetProcAddress(h, "RequestPluginAPI");       // 0x18002923a
//     IVPrismaUI1* api = request(1);                              // 0x18002924f
//
// Two DLLs on disk bracket the interface:
//
//   * `SexLabPPrism.dll`            — the ORIGINAL consumer we reconstruct
//                                     (artifacts/SexLabPPrism.dll). Its
//                                     OnMessage (0x180029200) is the only place
//                                     it touches the vtable:
//                                       call *0x00  (iface, "SexLabPPrism/controller-0.6.1.html", cb)
//                                       call *0x70  (iface, view, 0x50)
//                                       call *0xa8  (iface, view, OnConsoleMessage)
//                                       call *0x18  (iface, view, name, cb)   x6
//                                       call *0x40  (iface, view)
//                                     (recon/decompiled/0x180029200_OnMessage.c)
//
//   * `PrismaUITeleportMenu.dll`    — a WORKING CONSUMER of the same runtime,
//                                     shipped at
//                                     `PrismaUI Teleport Menu (v1.6.0)/SKSE/Plugins/PrismaUITeleportMenu.dll`.
//                                     It resolves the same two globals:
//                                       0x1800e90a8 = request(0)   // base interface
//                                       0x1800e90b0 = request(1)   // "V2" interface (console)
//                                       0x1800e90b8 = first view
//                                       0x1800e90c8 = second view
//                                     and reveals every slot below with its
//                                     argument registers. File offset of the
//                                     disassembly used: `tele.dis`, evidence
//                                     call sites cited per slot.
//
// Call convention for every slot is `__thiscall`-style MSVC x64: rcx = `this`
// (the interface pointer), then rdx, r8, r9 = arguments, extra args on the
// stack. Confirmed in both DLLs by `mov (%rcx),%rax; call *0xNN(%rax)`.
//
// IMPORTANT — evidence levels:
//   CONFIRMED  = the ORIGINAL DLL calls this offset itself (0x00/0x18/0x40/
//                0x70/0xa8), so we are certain it exists on the interface
//                returned by RequestPluginAPI(1).
//   CROSS-CHECK= only PrismaUITeleportMenu calls it, on its RequestPluginAPI(0)
//                handle. Strong evidence of the slot and its shape, but the
//                offset is proven on the BASE interface, not on apiVersion 1.
//   UNKNOWN    = no call site recovered anywhere. Do not call it.
//
// The interface returned by RequestPluginAPI(1) is named
// `PRISMA_UI_API::IVPrismaUI1` in the original's own RTTI-adjacent strings
// (FocusRecovery::Begin, recon/strings.txt:75). The base handle is unnamed.

#include <cstdint>

namespace PrismaUI::vtbl
{
	// --- byte offsets (vtable index = offset / 8) ---------------------------
	inline constexpr std::size_t kCreateView      = 0x00;  // CONFIRMED (both)
	inline constexpr std::size_t kExecuteJs       = 0x08;  // CROSS-CHECK
	inline constexpr std::size_t kInvokeFunction  = 0x10;  // CROSS-CHECK
	inline constexpr std::size_t kRegisterCallback= 0x18;  // CONFIRMED (both)
	inline constexpr std::size_t kSetInteractive  = 0x28;  // CROSS-CHECK
	inline constexpr std::size_t kSlot30          = 0x30;  // CROSS-CHECK, name UNKNOWN
	inline constexpr std::size_t kQueryFocus      = 0x38;  // CROSS-CHECK
	inline constexpr std::size_t kApplyView       = 0x40;  // CONFIRMED (both)
	inline constexpr std::size_t kCloseView       = 0x60;  // CROSS-CHECK
	inline constexpr std::size_t kSetViewFlags    = 0x70;  // CONFIRMED (original)
	inline constexpr std::size_t kSlot80          = 0x80;  // UNKNOWN
	inline constexpr std::size_t kSlot88          = 0x88;  // UNKNOWN (old guess; retired)
	inline constexpr std::size_t kIsConnected     = 0xa0;  // CROSS-CHECK
	inline constexpr std::size_t kSetConsoleSink  = 0xa8;  // CONFIRMED (both, api v1)

	// --- CONFIRMED SLOTS ----------------------------------------------------

	// 0x00 — CreateView. Returns the opaque view handle.
	//   original 0x18002927e: call *[vtbl]      (rcx=iface, rdx="SexLabPPrism/controller-0.6.1.html", r8=&lambda_1)
	//   teleport 0x180054b4a: call *(%rax)      (rcx=iface, rdx="PrismaUITeleportMenu/index.html", r8=0x18003d8c0)
	// The callback is `CreateViews::<lambda_1>::operator()(unsigned __int64)`
	// (recon/strings.txt:45) — it takes the view handle back.
	using CreateViewFn = void* (*)(void* a_self, const char* a_path, void* a_onCreated);
	inline constexpr char kCreateViewSig[] = "void*(*,const char*,void*)";

	// 0x18 — RegisterCallback (JS -> C++). Both DLLs register 3-4 byte names.
	//   original 0x1800292dd: call *0x18(%rax)  (rcx=iface, rdx=view, r8=name, r9=&trampoline) x6
	//   teleport 0x180054b86: call *0x18(%rax)  (rcx=iface, rdx=view, r8="selectLocation", r9=0x18003d8d0)
	// Original names, in registration order (recon/decompiled/0x180029200_OnMessage.c):
	//   "slppReady", "slppAction", "slppSearchRequest", "slppCollapsed",
	//   "slppLog", "slppCatalogRetry".
	using RegisterCallbackFn = void (*)(void* a_self, void* a_view, const char* a_name, void* a_callback);
	inline constexpr char kRegisterCallbackSig[] = "void(*,void*,const char*,void*)";

	// 0x40 — Apply view state after creation / on each transition.
	//   original 0x18002931a: call *0x40(%rax)  (rcx=iface, rdx=view)
	//   teleport 0x180054b64: call *r8 where r8=[vtbl+0x40] (rcx=iface, rdx=view)
	// Called right after CreateView+RegisterCallback (original) and right
	// after CreateView (teleport). Behaviour: commits/arms the view. The old
	// reconstruction called this "DOM-ready / show".
	using ApplyViewFn = void (*)(void* a_self, void* a_view);

	// 0x70 — Set view configuration. The original passes literal 0x50.
	//   original 0x1800292a4: call *0x70(%rax)  (rcx=iface, rdx=view, r8=0x50)
	// NOTE: teleport's two `call *0x70` sites (0x1800618c6, 0x180062456) are on
	// unrelated interfaces and do NOT validate this slot. Name and 0x50 meaning
	// (size? flags? transparency+focus?) are UNKNOWN; only (iface,view,int) is.
	using SetViewFlagsFn = void (*)(void* a_self, void* a_view, int a_value);

	// 0xa8 — Install the JS console-message sink (api version 1 only).
	//   original 0x1800292bb: call *0xa8(%rax)  (rcx=iface, rdx=view, r8=&OnConsoleMessage)
	//   teleport 0x180049669: call *0xa8(%rax)  (rcx=request(1) iface, rdx=arg1, r8=0x18003d7e0)
	// The handler is `OnConsoleMessage(unsigned __int64, ConsoleMessageLevel, const char*)`
	// (recon/strings.txt:88).
	using SetConsoleSinkFn = void (*)(void* a_self, void* a_view, void* a_callback);

	// --- CROSS-CHECK SLOTS (proven on the base interface) -------------------

	// 0x08 — Execute a JS *expression/snippet*. r9 is an optional callback.
	//   teleport 0x180046fe8: call *0x8(%rax)   (rcx=iface, rdx=view, r8="endScreenshot()", r9=0)
	//   teleport 0x18004cfe5: call *0x8(%rax)   (rcx=iface, rdx=view, r8="locationsChanged()", r9=0)
	//   teleport 0x18004a254: call *0x8(%rax)   (rcx=iface, rdx=view, r8="onImmersiveClose()", r9=0)
	//   teleport 0x18004a32a: call *0x8(%rax)   (rcx=iface, rdx=view, r8="onMenuClose()", r9=0)
	// NUL-terminated code string; no return used.
	using ExecuteJsFn = void (*)(void* a_self, void* a_view, const char* a_code, void* a_callback);

	// 0x10 — Invoke a named global JS function with one string argument.
	//   teleport 0x180046473: call *0x10(%rax)  (rcx=iface, rdx=view, r8="setTheme", r9=&std::string)
	//   teleport 0x180051e41: call *0x10(%rax)  (rcx=iface, rdx=view, r8="setTheme", r9=string-arg)
	// r9 points at a std::string (SSO-capable, `.data()` selected by the usual
	// `cmova` before the call). The argument is likely JSON for object-taking
	// functions; the interface itself does not care.
	using InvokeFunctionFn = void (*)(void* a_self, void* a_view, const char* a_name, const char* a_arg);

	// 0x28 — Set view interactive / input ownership.
	//   teleport 0x180050fe8: call *0x28(%rax)  (rcx=iface, rdx=view, r8=0, r9=0)
	//   teleport 0x18005111c: call *0x28(%rax)  (rcx=iface, rdx=view, r8b=1, r9=0)
	// Called on close as `false` and on open as `true` around the
	// onMenuClose()/onMenuOpen() JS notifications.
	using SetInteractiveFn = void (*)(void* a_self, void* a_view, bool a_interactive, void* a_unused);

	// 0x30 — (iface, view), no other args, return ignored. Name UNKNOWN.
	//   teleport 0x18004a268 / 0x18004a33e: call *0x30(%rax)  (rcx=iface, rdx=view)
	// Paired with 0x08("onMenuClose()") before the 0x40 tail-jump.
	using Slot30Fn = void (*)(void* a_self, void* a_view);

	// 0x38 — (iface, view) query, return ignored at both sites. Likely
	// "is view focused/visible". Name UNKNOWN.
	//   teleport 0x180050fe5 / 0x180051119: call *0x38(%rax)  (rcx=iface, rdx=view)
	using QueryFocusFn = void (*)(void* a_self, void* a_view);

	// 0x60 — Close/destroy the view; returns bool (true = it owned it).
	//   teleport Teleport_Close (0x18005bb2a) and Teleport_Open (0x18005bbfa)
	//   call *0x60(%rax)  (rcx=iface, rdx=view), then `test %al,%al`.
	using CloseViewFn = bool (*)(void* a_self, void* a_view);

	// 0xa0 — (iface) only, returns bool. Likely "runtime connected / ready".
	//   teleport 0x180057f23: call *0xa0(%rax)  (rcx=iface), `test %al,%al`.
	using IsConnectedFn = bool (*)(void* a_self);

	// --- reserved / never observed -----------------------------------------
	//   0x20, 0x48, 0x50, 0x58, 0x78, 0x80, 0x88, 0x90, 0x98, 0xb0+ : UNKNOWN.
	// The old reconstruction used 0x88 for "interop call"; there is NO evidence
	// for 0x88. It is retired in favour of 0x08/0x10 above.

	// Convenience: vtable index from byte offset.
	constexpr std::size_t Index(std::size_t a_byteOffset) { return a_byteOffset / sizeof(void*); }
}  // namespace PrismaUI::vtbl