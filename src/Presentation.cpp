#include "Presentation.h"

#include "PCH.h"
#include "PrismaUI.h"
#include "SceneState.h"

#include <functional>
#include <string>

namespace Presentation
{
	namespace
	{
		// Both apply functions operate on the vanilla scaleform menus. The
		// Prisma view itself is not a Scaleform menu, so it is untouched here —
		// which is also how the UIExtensions "CustomMenu" stays available: this
		// code never removes or hides a third-party menu, only the two vanilla
		// ones (HUD / Console). Video/scaleform menu names come from
		// IMenu::MENU_NAME (RE::HUDMenu::MENU_NAME == "HUD Menu").
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
	}  // namespace

	void ApplyVanillaHUDVisibility(bool a_sceneActive)
	{
		// The original hides the HUD while a scene is active and restores it
		// afterwards, and re-applies the same rule whenever the HUD menu opens
		// (recon/NATIVES-RECOVERED.md §4.3).
		SetMenuVisible(RE::HUDMenu::MENU_NAME, !a_sceneActive);
		logger::debug("Developer Console movie visibility restored: {}", !a_sceneActive);
	}

	void ApplyConsoleVisibility(bool a_sceneActive)
	{
		// The console must not be usable mid-scene; keep its movie hidden while
		// the scene is active. GUESS: the original also resolves Console::Hide()
		// through the address library; this menu-visibility rule is the CLNG
		// equivalent available without hand-resolved relocation ids.
		SetMenuVisible(RE::Console::MENU_NAME, !a_sceneActive);
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