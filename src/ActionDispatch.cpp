#include "ActionDispatch.h"

#include "Catalog.h"
#include "PCH.h"
#include "PrismaUI.h"
#include "SceneState.h"
#include "UiBridge.h"

#include <cstdlib>
#include <string>
#include <string_view>

// 0x18002c0d0, 88 instructions: the `slppSearchRequest` JS callback. It lives in
// the original's global anonymous namespace (src\main.cpp), so it is defined at
// file scope here. It only fires while a scene is active in camera mode with
// the interface shown and the Prisma bridge connected; the modal-search latch
// (DAT_18009c1cb) makes it idempotent.
namespace
{
	// `action|arg` split of a JS action payload. The original's HandleAction uses
	// these two helpers at file scope (they live in the unity TU's GLOBAL
	// anonymous namespace, not in a nested `ActionDispatch::anonymous-namespace`).
	std::string_view ActionOf(std::string_view a_payload)
	{
		const auto bar = a_payload.find('|');
		return bar == std::string_view::npos ? a_payload : a_payload.substr(0, bar);
	}

	std::string_view ArgOf(std::string_view a_payload)
	{
		const auto bar = a_payload.find('|');
		return bar == std::string_view::npos ? std::string_view{} : a_payload.substr(bar + 1);
	}

	void RequestSearchInput(const char* a_currentText)
	{
		if (!(SceneState::IsSceneActive() && !SceneState::IsUiMode() &&
				!SceneState::IsInterfaceHidden() && PrismaUI::IsAvailable())) {
			return;
		}
		if (SceneState::IsModalSearchOpen()) {
			logger::info("Modal search request ignored: menu already open");
			return;
		}
		SceneState::SetModalSearchOpen(true);
		const std::string text{ a_currentText ? a_currentText : "" };
		auto* const       task = SKSE::GetTaskInterface();
		if (task) {
			// Original queues a lambda that drives the Prisma search UI; the JS
			// entry point it calls was not extracted (marked gap).
			task->AddTask([text]() { PrismaUI::InvokeJs("slppOpenSearch", text); });
		}
	}
}  // namespace

namespace ActionDispatch
{
	void SendModEvent(const char* a_eventName, const char* a_strArg, float a_numArg)
	{
		auto* const source = SKSE::GetModCallbackEventSource();
		if (!source) {
			logger::warn("Mod callback event source unavailable; '{}' not dispatched", a_eventName);
			return;
		}
		SKSE::ModCallbackEvent event{};
		event.eventName = a_eventName;
		event.strArg    = a_strArg ? a_strArg : "";
		event.numArg    = a_numArg;
		event.sender    = nullptr;
		source->SendEvent(&event);
	}

	void HandleAction(const char* a_payload)
	{
		if (!a_payload) {
			return;
		}
		const std::string_view payload{ a_payload };
		const std::string_view action = ActionOf(payload);

		// The controller script is the authority for every scene action; forward
		// the raw payload verbatim so `OnPrismAction` parses action|arg exactly as
		// the PEX expects (recon/PAPYRUS-CONTRACT.md §2).
		logger::info("UI action received: {}", payload);
		logger::info("UI action dispatched to Papyrus: {}", action);
		SendModEvent("SLPPPrism_Action", a_payload, 0.0F);
	}

	void HandleSearchRequest(const char* a_currentText)
	{
		// The registered JS trampoline; the recovered body is the file-scope
		// RequestSearchInput (see above).
		::RequestSearchInput(a_currentText);
	}

	void SetCollapsed(bool a_collapsed, const char* a_source)
	{
		logger::info("Collapse state -> {} (via {})", a_collapsed, a_source ? a_source : "");
		// The native side owns collapse state (controller-0.6.1.html comment) and
		// reflects it back through window.slppSetCollapsed.
		PrismaUI::InvokeJs("slppSetCollapsed", a_collapsed ? "1" : "0");
	}

	void HandleReady(const char* a_arg)
	{
		logger::info("Controller view JS handshake received");
		if (a_arg && a_arg[0] != '\0') {
			logger::debug("slppReady arg: {}", a_arg);
		}
		// Re-send the current scene state so a view that finished loading after
		// the last PublishSceneState still renders correctly. PushState() re-reads
		// the published global itself, so no payload is passed.
		UiBridge::PushState();
	}

	void HandleLog(const char* a_message)
	{
		logger::info("[UI] {}", a_message ? a_message : "");
	}

	void HandleCatalogRetry(const char* a_arg)
	{
		int attempt = 0;
		if (a_arg && a_arg[0] != '\0') {
			attempt = std::atoi(a_arg);
			if (attempt < 0) {
				attempt = 0;
			}
		}
		Catalog::RetryPublish(attempt);
	}
}  // namespace ActionDispatch