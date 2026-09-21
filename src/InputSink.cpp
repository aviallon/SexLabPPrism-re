#include "InputSink.h"

#include "PCH.h"
#include "FocusRecovery.h"
#include "Presentation.h"
#include "PrismaUI.h"
#include "SceneState.h"

// The original's sink class is `?AVInputSink@?A0xbb2e73b6@@`: a TOP-LEVEL class
// named `InputSink` in the unity TU's anonymous namespace. Its ProcessEvent
// bakes the __FUNCSIG__
//   `enum RE::BSEventNotifyControl __cdecl `anonymous-namespace'::InputSink::ProcessEvent(...)`
// (recon/decompiled-new/0x18001f0b0___AVInputSink__A0xbb2e73b6____vfunc_1_.c).
// A nested `InputSink::`anonymous-namespace'::Sink` yields the wrong RTTI name
// and the wrong FUNCSIG, so the class lives in this TU's GLOBAL anon namespace.
namespace
{
	// Scan codes (DIK_*) from the decompilation: 0x29 = grave/console key,
	// 0x3E = F4, 0x3D = F3.
	constexpr std::uint32_t kConsoleKey = 0x29;
	constexpr std::uint32_t kUiKey      = 0x3E;
	constexpr std::uint32_t kHideKey    = 0x3D;

	class InputSink final : public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		RE::BSEventNotifyControl ProcessEvent(
			RE::InputEvent* const*              a_events,
			RE::BSTEventSource<RE::InputEvent*>*) override
		{
			if (!a_events) {
				return RE::BSEventNotifyControl::kContinue;
			}

			for (auto* event = *a_events; event; event = event->next) {
				const auto* const button = event->AsButtonEvent();
				if (!button ||
					button->GetDevice() != RE::INPUT_DEVICE::kKeyboard ||
					!button->IsDown()) {
					continue;
				}

				const auto key = button->GetIDCode();

				// The console hotkey is consumed outright during a scene and
				// never reaches the game (0x18001f0b0:0x2d0).
				if (key == kConsoleKey && SceneState::IsSceneActive()) {
					logger::info("Developer Console hotkey suppressed during active scene");
					return RE::BSEventNotifyControl::kStop;
				}

				// F3/F4 are ignored while the search box owns the keyboard.
				if (SceneState::IsModalSearchOpen()) {
					continue;
				}

				if (key == kUiKey) {
					if (!SceneState::IsSceneActive()) {
						continue;
					}
					// 0x18001f285: the label is chosen from the PRE-toggle value
					// (the `xchg` stores the new bit and returns the old one, and
					// `cmove` selects on `%bl`), so "camera mode" is logged on
					// the camera->UI transition, not the new UI state.
					const bool wasUiMode = SceneState::IsUiMode();
					const bool uiMode    = !wasUiMode;
					SceneState::SetUiMode(uiMode);
					logger::info("F4: {}", wasUiMode ? "UI mode (Prisma focused)" : "camera mode (Prisma released)");
					Presentation::QueuePresentation();
					if (!wasUiMode) {
						// Original: Begin(DAT_18009c1b0, DAT_18009c1c0, &sceneActive,
						// &uiMode) at 0x18001f285. The two atomics are owned by
						// SceneState and not address-exposed yet, so nullptr is passed
						// and the state machine reads the SceneState predicates instead
						// (marked gap).
						FocusRecovery::Begin(PrismaUI::Interface(), reinterpret_cast<std::uint64_t>(PrismaUI::View()), nullptr, nullptr);
					} else {
						FocusRecovery::Cancel();
					}
					continue;
				}

				if (key == kHideKey && SceneState::IsSceneActive()) {
					const bool hidden = SceneState::ToggleInterfaceHidden();
					logger::info("F3: interface {}", hidden ? "fully hidden" : "restored");
					Presentation::ApplyPresentation();
				}
			}

			return RE::BSEventNotifyControl::kContinue;
		}
	};

	InputSink g_sink;
}  // namespace

namespace PrismSinks
{
	void RegisterInput()
	{
		auto* const manager = RE::BSInputDeviceManager::GetSingleton();
		if (!manager) {
			logger::warn("BSInputDeviceManager unavailable; InputSink not registered");
			return;
		}
		manager->AddEventSink(&g_sink);
		logger::info("InputSink registered on the input event source");
	}
}  // namespace PrismSinks
