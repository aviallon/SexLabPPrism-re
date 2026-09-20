#include "UiBridge.h"

#include "PCH.h"

#include <functional>
#include <mutex>

// See UiBridge.h: this is the ONLY translation unit that knows anything about
// the external PrismaUI.dll ABI. The exact IVPrismaUI1 vtable layout was not
// recoverable from the original binary (recon/NATIVES-RECOVERED.md §6), so the
// dereference of the interface is isolated in InvokeJs() and guarded by
// kPrismaUiLayoutConfirmed.
namespace UiBridge
{
	namespace
	{
		constexpr std::string_view kPluginDll   = "PrismaUI.dll"sv;
		constexpr std::string_view kExportName  = "RequestPluginAPI"sv;
		constexpr std::string_view kViewPath    = "SexLabPPrism/controller-0.6.1.html"sv;
		constexpr int              kApiVersion  = 1;

		// --- external ABI shape (guessed where marked) -------------------------
		// void* __cdecl RequestPluginAPI(int version);  [recovered: OnMessage]
		using RequestPluginApiFn = void*(*)(int);

		// The interface's CreateView/RegisterCallback/InteropCall slots are only
		// known to exist, not their order. This flag documents that calls through
		// the interface are deliberately disabled until the layout is nailed down.
		constexpr bool kPrismaUiLayoutConfirmed = false;

		// GUESS: slot index (in pointers) of InteropCall(view, functionName, arg).
		// PrismaUI exposes view-scoped JS invocation but the vtable offsets in the
		// original (0x00 CreateView, 0x18 RegisterCallback, 0x40 DOM-ready,
		// 0x70 flags, 0xa8 console handler) do not include an obvious slot and the
		// remaining ones were not traced. Replace this constant only.
		constexpr std::size_t kInteropCallVtblIndex = 0x07;

		using InteropCallFn = void (*)(void* a_view, const char* a_function, const char* a_argument);

		struct Api
		{
			void*              iface   = nullptr;
			void*              view    = nullptr;
			RequestPluginApiFn request = nullptr;
		};

		Api            g_api;
		std::once_flag g_loadOnce;

		void LoadApi()
		{
			const auto mod = REX::W32::GetModuleHandleW(L"PrismaUI.dll");
			if (!mod) {
				logger::warn("PrismaUI.dll not loaded; SexLab P+ Prism UI bridge inactive");
				return;
			}
			const auto request = reinterpret_cast<RequestPluginApiFn>(
				REX::W32::GetProcAddress(mod, kExportName.data()));
			if (!request) {
				logger::warn("PrismaUI.dll has no {} export; UI bridge inactive", kExportName);
				return;
			}
			g_api.request = request;
			g_api.iface   = request(kApiVersion);
			if (!g_api.iface) {
				logger::warn("PrismaUI RequestPluginAPI({}) returned null", kApiVersion);
				return;
			}
			// View creation (CreateView(kViewPath, ...)) belongs to OnMessage, which is
			// not part of this reconstruction step; without it there is no view handle
			// to invoke against. Kept here so the load path matches the original.
			logger::info("PrismaUI API v{} acquired (view '{}' not created yet)", kApiVersion, kViewPath);
		}

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
		std::call_once(g_loadOnce, LoadApi);
		return g_api.iface != nullptr && g_api.view != nullptr;
	}

	void InvokeJs(const char* a_functionName, std::string_view a_argument)
	{
		std::call_once(g_loadOnce, LoadApi);
		if (!g_api.iface || !g_api.view) {
			static bool warned = false;
			if (!warned) {
				warned = true;
				logger::warn("PrismaUI bridge not connected; JS callback '{}' will not run", a_functionName);
			}
			return;
		}
		if (!kPrismaUiLayoutConfirmed) {
			logger::warn("PrismaUI interface layout unconfirmed; JS callback '{}' ({}-byte arg) skipped",
				a_functionName, a_argument.size());
			return;
		}
		// The single external-vtable dereference in the whole plugin.
		const auto* const vtbl = *reinterpret_cast<void* const* const*>(g_api.iface);
		const auto        call = reinterpret_cast<InteropCallFn>(vtbl[kInteropCallVtblIndex]);
		const std::string arg{ a_argument };
		call(g_api.view, a_functionName, arg.c_str());
	}

	void PushState(std::string_view a_json)
	{
		const std::string json{ a_json };
		QueueOnGameThread([json]() { InvokeJs("slppState", json); });
	}

	void PushCompatible(std::string_view a_json)
	{
		const std::string json{ a_json };
		QueueOnGameThread([json]() { InvokeJs("slppSetCompatible", json); });
	}

	void SetSearchQuery(std::string_view a_query)
	{
		const std::string query{ a_query };
		QueueOnGameThread([query]() { InvokeJs("slppSetSearchQuery", query); });
	}

	void PushCatalog(std::string_view a_rowsJson, std::int32_t a_total)
	{
		const std::string rows{ a_rowsJson };
		const auto        total = std::to_string(a_total);
		QueueOnGameThread([rows, total]() {
			InvokeJs("slppCatalogReset", total);
			InvokeJs("slppCatalogChunk", rows);
			InvokeJs("slppCatalogDone", total);
		});
	}
}  // namespace UiBridge