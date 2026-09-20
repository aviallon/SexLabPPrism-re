#include "PrismaUI.h"

#include "ActionDispatch.h"
#include "PCH.h"

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

		// --- recovered IVPrismaUI1 vtable byte offsets (OnMessage) -------------
		constexpr std::size_t kCreateViewVtblOffset       = 0x00;
		constexpr std::size_t kRegisterCallbackVtblOffset = 0x18;
		constexpr std::size_t kDomReadyVtblOffset         = 0x40;
		constexpr std::size_t kViewFlagsVtblOffset        = 0x70;
		constexpr std::size_t kConsoleHandlerVtblOffset   = 0xa8;

		// NOT recovered (recon/NATIVES-RECOVERED.md §6): the slot used to invoke
		// JS from C++. Kept as a named constant so there is exactly one place to
		// correct it, and gated by kLayoutConfirmed so we never mis-call it.
		constexpr std::size_t kInteropCallVtblOffset = 0x88;
		constexpr bool        kLayoutConfirmed       = false;

		// void* __cdecl RequestPluginAPI(int version)
		using RequestPluginApiFn = void* (*)(int);

		// GUESSED ABI (slot order recovered, signatures are not): CreateView
		// receives the page path and a completion callback; RegisterCallback
		// receives (view, name, trampoline). Marked because only the offsets are
		// evidence-backed.
		using CreateViewFn = void* (*)(void* a_self, const char* a_path, void* a_callback);
		using RegisterCallbackFn = void (*)(void* a_self, void* a_view, const char* a_name, JsCallback a_cb);
		using ViewFlagsFn = void (*)(void* a_self, void* a_view, int a_flags);
		using ConsoleHandlerFn = void (*)(void* a_self, void* a_view, ConsoleMessageCallback a_cb);
		using DomReadyFn = void (*)(void* a_self, void* a_view);
		using InteropCallFn = void (*)(void* a_self, void* a_view, const char* a_function, const char* a_argument);

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

		// CreateViews::<lambda_1>: the view-ready completion. Its exact ABI is
		// not recovered; it only logs in the original, so keep it inert.
		void OnViewCreated(void* /*a_view*/, void* /*a_userdata*/)
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

		const auto mod = REX::W32::GetModuleHandleW(L"PrismaUI.dll");
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

		const auto createView = Slot<CreateViewFn>(iface, kCreateViewVtblOffset);
		void*      view       = createView ? createView(iface, kViewPath.data(), nullptr) : nullptr;
		g_state.view          = view;
		if (!view) {
			logger::error("PrismaUI CreateView failed for {}", kViewPath);
			return false;
		}

		if (const auto setFlags = Slot<ViewFlagsFn>(iface, kViewFlagsVtblOffset)) {
			setFlags(iface, view, 0x50);
		}
		if (const auto setConsole = Slot<ConsoleHandlerFn>(iface, kConsoleHandlerVtblOffset)) {
			setConsole(iface, view, OnConsoleMessage);
		}

		const auto registerCallback = Slot<RegisterCallbackFn>(iface, kRegisterCallbackVtblOffset);
		if (registerCallback) {
			registerCallback(iface, view, "slppReady", OnSlppReady);
			registerCallback(iface, view, "slppAction", ActionDispatch::HandleAction);
			registerCallback(iface, view, "slppSearchRequest", ActionDispatch::HandleSearchRequest);
			registerCallback(iface, view, "slppCollapsed", OnSlppCollapsed);
			registerCallback(iface, view, "slppLog", OnSlppLog);
			registerCallback(iface, view, "slppCatalogRetry", OnSlppCatalogRetry);
		} else {
			logger::error("PrismaUI RegisterCallback slot unavailable; JS callbacks not wired");
		}

		if (const auto domReady = Slot<DomReadyFn>(iface, kDomReadyVtblOffset)) {
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
		if (!kLayoutConfirmed) {
			static bool warned = false;
			if (!warned) {
				warned = true;
				logger::warn("Invoke skipped, view {} not usable ({} bytes)", a_functionName, a_argument.size());
			}
			return;
		}
		if (const auto call = Slot<InteropCallFn>(g_state.iface, kInteropCallVtblOffset)) {
			const std::string arg{ a_argument };
			call(g_state.iface, g_state.view, a_functionName, arg.c_str());
		}
	}

	bool IsFocused()
	{
		// The PrismaUI focus query slot was not recovered; report "unknown" as
		// not focused so FocusRecovery does not spin on a false positive.
		return false;
	}

	bool Unfocus()
	{
		// Same unknown-slot caveat as InvokeJs. Returns false = "could not
		// confirm an unfocus was issued", the conservative branch.
		return false;
	}
}  // namespace PrismaUI