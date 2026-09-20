#pragma once

// Presentation — HUD/console visibility and the Prisma "interactive" state.
//
// Recovered from recon/BINARY-RECON.md §1.4 and recon/NATIVES-RECOVERED.md
// §4.2–4.3:
//   ApplyVanillaHUDVisibility(bool sceneActive)  0x180026790
//   ApplyConsoleVisibility(bool sceneActive)     0x180025fc0
//   ApplyPresentation()                          0x1800261e0
//   QueuePresentation()                          queued on the SKSE TaskInterface
//
// MenuVisibilitySink calls the two visibility helpers around HUD/Console
// open-close events; InputSink's F3 path and F4 path call ApplyPresentation /
// QueuePresentation.
namespace Presentation
{
	// Hide (sceneActive == true) or restore the vanilla HUD movie.
	void ApplyVanillaHUDVisibility(bool a_sceneActive);

	// Keep the developer console consistent with the scene state.
	void ApplyConsoleVisibility(bool a_sceneActive);

	// Re-derive and apply the whole presentation from the shared SceneState
	// (HUD + console + Prisma interactivity).
	void ApplyPresentation();

	// ApplyPresentation() marshalled onto the game thread.
	void QueuePresentation();
}  // namespace Presentation