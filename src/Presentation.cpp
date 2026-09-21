#include "Presentation.h"

#include "PCH.h"
#include "PrismaUI.h"
#include "SceneState.h"

#include <functional>
#include <string>
#include <string_view>

// 0x180025fc0 lives in the original's global anonymous namespace (src\main.cpp),
// so its reconstructed body is defined here at file scope — that is what makes
// the compiler bake the ``anonymous-namespace'::ApplyConsoleVisibility``
// __FUNCSIG__ the matcher recovers.
namespace
{
	void SetMenuVisible(std::string_view a_menuName, bool a_visible)
	{
		auto* const ui = RE::UI::GetSingleton();
		if (!ui) {
			return;
		}
		const auto menu = ui->GetMenu(a_menuName);
		if (menu) {
			menu->uiMovie->SetVisible(a_visible);
		}
	}

	// 0x180025fc0, 126 instructions. SINGLE bool argument (the original's
	// __FUNCSIG__ is `void __cdecl `anonymous-namespace'::ApplyConsoleVisibility(bool)`;
	// the "Console" name is hard-coded inside, not passed in — the extra
	// registers at the MenuVisibilitySink call site are leftovers). It keeps the
	// last visible-state in DAT_180095122 and a "hidden by scene" latch in
	// DAT_18009c1ce so the pre-scene visibility is restored exactly.
	bool g_consoleHiddenByScene = false;
	bool g_savedConsoleVisible   = false;

	void ApplyConsoleVisibility(bool a_sceneActive)
	{
		auto* const ui = RE::UI::GetSingleton();
		if (!ui) {
			return;
		}
		const auto menu = ui->GetMenu("Console"sv);
		if (!menu || !menu->uiMovie) {
			return;
		}
		if (!a_sceneActive) {
			if (g_consoleHiddenByScene) {
				menu->uiMovie->SetVisible(g_savedConsoleVisible);
				g_consoleHiddenByScene = false;
				logger::info("Developer Console movie visibility restored: {}", g_savedConsoleVisible);
			}
		} else {
			if (!g_consoleHiddenByScene) {
				g_savedConsoleVisible   = menu->uiMovie->GetVisible();
				g_consoleHiddenByScene = true;
			}
			if (menu->uiMovie->GetVisible()) {
				menu->uiMovie->SetVisible(false);
				logger::info("Developer Console movie hidden during scene");
			}
		}
	}
}  // namespace

namespace Presentation
{
	void ApplyVanillaHUDVisibility(bool a_sceneActive)
	{
		// The original hides the HUD while a scene is active and restores it
		// afterwards, and re-applies the same rule whenever the HUD menu opens
		// (recon/NATIVES-RECOVERED.md §4.3).
		SetMenuVisible(RE::HUDMenu::MENU_NAME, !a_sceneActive);
		logger::debug("Vanilla HUD movie visibility restored: {}", !a_sceneActive);
	}

	void ApplyConsoleVisibility(bool a_sceneActive)
	{
		// The console must not be usable mid-scene; keep its movie hidden while
		// the scene is active. Delegates to the file-scope recovered body.
		::ApplyConsoleVisibility(a_sceneActive);
	}

	void ApplyPresentation()
	{
		const bool sceneActive = SceneState::IsSceneActive();
		ApplyVanillaHUDVisibility(sceneActive);
		ApplyConsoleVisibility(sceneActive);

		// "not-interactive" in controller-0.6.1.html is driven by
		// window.slppSetInteractive: true while the UI owns the cursor (F4 UI
		// mode), false while the camera does.
		const bool interactive = sceneActive && SceneState::IsUiMode();
		PrismaUI::InvokeJs("slppSetInteractive", interactive ? "1" : "0");
	}

	void QueuePresentation()
	{
		auto* const task = SKSE::GetTaskInterface();
		if (!task) {
			ApplyPresentation();
			return;
		}
		task->AddTask([]() { ApplyPresentation(); });
	}
}  // namespace Presentation