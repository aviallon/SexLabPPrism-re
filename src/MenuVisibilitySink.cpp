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
				RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;
		};

		// The real body, kept separate so the structure mirrors the original's
		// thin ProcessEvent thunk + helper calls.
		RE::BSEventNotifyControl Handle(const RE::MenuOpenCloseEvent* a_event)
		{
			if (!a_event) {
				return RE::BSEventNotifyControl::kContinue;
			}

			const std::string_view menuName{ a_event->menuName };
			const bool             sceneActive = SceneState::IsSceneActive();

			if (menuName == RE::HUDMenu::MENU_NAME) {
				if (a_event->opening) {
					Presentation::ApplyVanillaHUDVisibility(sceneActive);
				} else {
					Presentation::ApplyPresentation();
				}
				return RE::BSEventNotifyControl::kContinue;
			}

			if (menuName == RE::Console::MENU_NAME) {
				Presentation::ApplyConsoleVisibility(sceneActive);
				if (sceneActive && !a_event->opening) {
					// The original additionally resolves Console::Hide() through
					// the address library here (recon/NATIVES-RECOVERED.md §4.3);
					// ApplyConsoleVisibility(true) is the CLNG-visible equivalent.
					Presentation::ApplyConsoleVisibility(true);
					logger::info("Developer Console closed during active scene");
				}
			}
			return RE::BSEventNotifyControl::kContinue;
		}

		RE::BSEventNotifyControl Sink::ProcessEvent(
			const RE::MenuOpenCloseEvent* a_event,
			RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
		{
			return Handle(a_event);
		}
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