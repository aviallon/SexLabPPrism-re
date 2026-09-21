#include "PrismaUI.h"

#include "ActionDispatch.h"
#include "PCH.h"
#include "PrismaUI_vtbl.h"

#include <cstring>
#include <string>

namespace PrismaUI
{
	namespace
	{
		constexpr std::string_view kPluginDll  = "PrismaUI.dll"sv;
		constexpr std::string_view kExportName = "RequestPluginAPI"sv;
		constexpr std::string_view kViewPath   = "SexLabPPrism/controller-0.6.1.html"sv;
		constexpr int              kApiVersion = 1;

		// Recovered IVPrismaUI1 layout: see PrismaUI_vtbl.h for the evidence
		// (offset, argument registers, cross-check call sites in
		// PrismaUITeleportMenu.dll). Never infer an offset from a name here.
		namespace V = vtbl;

		// The layout is confirmed for every slot the original itself calls
		// (0x00/0x18/0x40/0x70/0xa8) and cross-checked for the JS invocation
		// slots (0x08/0x10). InvokeJs may therefore call the interface now.
		constexpr bool kLayoutConfirmed = true;

		// void* __cdecl RequestPluginAPI(int version)
		using RequestPluginApiFn = void* (*)(int);

		using CreateViewFn = V::CreateViewFn;
		using RegisterCallbackFn = V::RegisterCallbackFn;
		using ViewFlagsFn = V::SetViewFlagsFn;
		using ConsoleHandlerFn = V::SetConsoleSinkFn;
		using DomReadyFn = V::ApplyViewFn;
		using InvokeFunctionFn = V::InvokeFunctionFn;
		using ExecuteJsFn = V::ExecuteJsFn;
		using SetInteractiveFn = V::SetInteractiveFn;
		using QueryFocusFn = V::QueryFocusFn;

		struct State
		{
			void* iface = nullptr;
			void* view  = nullptr;
		};

		State   g_state;
		bool    g_created = false;

		template <class Fn>
		Fn Slot(void* a_iface, std::size_t a_byteOffset)
		{
			// The single legal dereference of the external interface vtable.
			auto** const vtbl = *static_cast<void***>(a_iface);
			void*        raw  = vtbl[a_byteOffset / sizeof(void*)];
			Fn           fn{};
			static_assert(sizeof(fn) == sizeof(raw));
			std::memcpy(&fn, &raw, sizeof(fn));
			return fn;
		}

		// --- JS trampolines (CreateViews::<lambda_N> in the original) ----------

		// CreateViews::<lambda_1>::operator()(unsigned __int64) const
		// (recon/strings.txt:45). The view-ready completion; it only logs in the
		// original, so it stays inert but keeps the recovered ABI.
		void OnViewCreated(std::uint64_t /*a_view*/)
		{
			logger::debug("Controller view created");
		}

		// OnConsoleMessage(uint64, ConsoleMessageLevel, const char*)
		void OnConsoleMessage(std::uint64_t, int a_level, const char* a_message)
		{
			logger::info("[PrismaUI console:{}] {}", a_level, a_message ? a_message : "");
		}

		void OnSlppReady(const char* a_arg)
		{
			ActionDispatch::HandleReady(a_arg);
		}
		void OnSlppLog(const char* a_arg)
		{
			ActionDispatch::HandleLog(a_arg);
		}
		void OnSlppCatalogRetry(const char* a_arg)
		{
			ActionDispatch::HandleCatalogRetry(a_arg);
		}
		void OnSlppCollapsed(const char* a_arg)
		{
			ActionDispatch::SetCollapsed(a_arg != nullptr && a_arg[0] != '0' && a_arg[0] != '\0', "slppCollapsed");
		}
	}  // namespace

	bool IsAvailable()
	{
		return g_state.iface != nullptr && g_state.view != nullptr;
	}

	void* Interface() { return g_state.iface; }
	void* View() { return g_state.view; }

	bool CreateViews()
	{
		if (g_created) {
			return IsAvailable();
		}
		g_created = true;

		// 0x180029220: GetModuleHandleA("PrismaUI.dll") — the original uses the
		// ANSI variant, not the wide one.
		const auto mod = REX::W32::GetModuleHandleA(kPluginDll.data());
		if (!mod) {
			logger::warn("PrismaUI API not found");
			return false;
		}
		const auto request = reinterpret_cast<RequestPluginApiFn>(
			REX::W32::GetProcAddress(mod, kExportName.data()));
		if (!request) {
			logger::warn("PrismaUI API not found");
			return false;
		}

		void* const iface = request(kApiVersion);
		if (!iface) {
			logger::warn("PrismaUI API not found");
			return false;
		}
		g_state.iface = iface;

		const auto createView = Slot<CreateViewFn>(iface, V::kCreateView);
		void*      view       = createView ?
		                  createView(iface, kViewPath.data(), reinterpret_cast<void*>(&OnViewCreated)) :
		                  nullptr;
		g_state.view          = view;
		if (!view) {
			logger::error("PrismaUI CreateView failed for {}", kViewPath);
			return false;
		}

		if (const auto setFlags = Slot<ViewFlagsFn>(iface, V::kSetViewFlags)) {
			setFlags(iface, view, 0x50);
		}
		if (const auto setConsole = Slot<ConsoleHandlerFn>(iface, V::kSetConsoleSink)) {
			setConsole(iface, view, reinterpret_cast<void*>(&OnConsoleMessage));
		}

		const auto registerCallback = Slot<RegisterCallbackFn>(iface, V::kRegisterCallback);
		if (registerCallback) {
			registerCallback(iface, view, "slppReady", reinterpret_cast<void*>(&OnSlppReady));
			registerCallback(iface, view, "slppAction", reinterpret_cast<void*>(&ActionDispatch::HandleAction));
			registerCallback(iface, view, "slppSearchRequest", reinterpret_cast<void*>(&ActionDispatch::HandleSearchRequest));
			registerCallback(iface, view, "slppCollapsed", reinterpret_cast<void*>(&OnSlppCollapsed));
			registerCallback(iface, view, "slppLog", reinterpret_cast<void*>(&OnSlppLog));
			registerCallback(iface, view, "slppCatalogRetry", reinterpret_cast<void*>(&OnSlppCatalogRetry));
		} else {
			logger::error("PrismaUI RegisterCallback slot unavailable; JS callbacks not wired");
		}

		if (const auto domReady = Slot<DomReadyFn>(iface, V::kApplyView)) {
			domReady(iface, view);
		}

		logger::info("Single controller/HUD view created: {}", kViewPath);
		return true;
	}

	void InvokeJs(const char* a_functionName, std::string_view a_argument)
	{
		if (!IsAvailable()) {
			static bool warned = false;
			if (!warned) {
				warned = true;
				logger::warn("PrismaUI bridge not connected; JS callback '{}' will not run", a_functionName);
			}
			return;
		}
		static_assert(kLayoutConfirmed, "PrismaUI vtable layout not confirmed");

		// C++ -> JS: slot 0x10 invokes a named global JS function with one
		// string argument (PrismaUI_vtbl.h kInvokeFunction, evidence
		// PrismaUITeleportMenu 0x180046473 / 0x180051e41, r8="setTheme").
		// The controller page defines the receiving functions as
		// `window.slppSetInteractive`, `window.slppState`, `window.slppVitals`, ...
		// (recon/controller-0.6.1.html), so the plain name is correct here.
		const std::string arg{ a_argument };
		if (const auto call = Slot<InvokeFunctionFn>(g_state.iface, V::kInvokeFunction)) {
			call(g_state.iface, g_state.view, a_functionName, arg.c_str());
		}
	}

	void ExecuteJs(const char* a_code)
	{
		// Slot 0x08: evaluate a JS expression (PrismaUI_vtbl.h kExecuteJs,
		// evidence PrismaUITeleportMenu 0x18004a32a r8="onMenuClose()").
		if (!IsAvailable()) {
			return;
		}
		if (const auto call = Slot<ExecuteJsFn>(g_state.iface, V::kExecuteJs)) {
			call(g_state.iface, g_state.view, a_code, nullptr);
		}
	}

	bool IsFocused()
	{
		// Slot 0x38: (iface, view) query, called by the working consumer around
		// its open/close path (PrismaUI_vtbl.h kQueryFocus, teleport
		// 0x180050fe5 / 0x180051119). The return convention was not recovered,
		// so the slot is invoked for its side effect and the answer is reported
		// conservatively; FocusRecovery only needs "was an unfocus issued".
		if (!IsAvailable()) {
			return false;
		}
		if (const auto query = Slot<QueryFocusFn>(g_state.iface, V::kQueryFocus)) {
			query(g_state.iface, g_state.view);
		}
		return false;
	}

	bool Unfocus()
	{
		// Slot 0x28: set the view non-interactive (PrismaUI_vtbl.h
		// kSetInteractive, teleport 0x180050fe8 calls it with r8=0). This is the
		// C++ side of the F4 camera transition: release keyboard/mouse to the
		// camera before FocusRecovery verifies cleanup.
		if (!IsAvailable()) {
			return false;
		}
		if (const auto setInteractive = Slot<SetInteractiveFn>(g_state.iface, V::kSetInteractive)) {
			setInteractive(g_state.iface, g_state.view, false, nullptr);
			return true;
		}
		return false;
	}
}  // namespace PrismaUI
