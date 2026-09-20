#include "MenuVisibilitySink.h"

#include "PCH.h"
#include "Presentation.h"
#include "SceneState.h"

#include <string_view>

namespace MenuVisibilitySink
{
	namespace
	{
		class Sink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
		{
		public:
			RE::BSEventNotifyControl ProcessEvent(
				const RE::MenuOpenCloseEvent* a_event,
				RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
			{
				// 0x18001f3c0: `test rdx,rdx / cmpb $0,0x8(%rdx)` — a null event or a
				// CLOSING event returns kContinue immediately, so the whole body is
				// an open-only path.
				if (!a_event || !a_event->opening) {
					return RE::BSEventNotifyControl::kContinue;
				}

				const bool sceneActive = SceneState::IsSceneActive();

				// HUD first: `movups "HUD Menu" / call 0x180023df0` then
				// `call 0x180026790` (ApplyVanillaHUDVisibility).
				if (a_event->menuName == RE::HUDMenu::MENU_NAME) {
					Presentation::ApplyVanillaHUDVisibility(sceneActive);
					return RE::BSEventNotifyControl::kContinue;
				}

				if (a_event->menuName == RE::Console::MENU_NAME) {
					Presentation::ApplyConsoleVisibility(sceneActive);
					if (sceneActive) {
						// The original additionally resolves Console::Hide() and a
						// method on the menu through the address library here
						// (recon/NATIVES-RECOVERED.md §4.3); ApplyConsoleVisibility
						// is the CLNG-visible equivalent.
						logger::info("Developer Console closed during active scene");
					}
				}
				return RE::BSEventNotifyControl::kContinue;
			}
		};
	}  // namespace

	void Register()
	{
		auto* const ui = RE::UI::GetSingleton();
		if (!ui) {
			logger::warn("RE::UI unavailable; MenuVisibilitySink not registered");
			return;
		}
		// A single shared sink instance registered on the UI event source.
		static Sink sink;
		ui->AddEventSink<RE::MenuOpenCloseEvent>(&sink);
		logger::info("MenuVisibilitySink registered on the UI event source");
	}
}  // namespace MenuVisibilitySink