#pragma once

// JS -> C++ callback trampolines registered on the PrismaUI view, plus the
// action dispatcher behind `slppAction`.
//
// The controller Papyrus script (SLP_PrismController) registers for two mod
// events and is the authority for every scene action:
//   "SLPPPrism_Action"         -> OnPrismAction(String payload, ...)
//   "SLPPPrism_SearchRequest"  -> OnSearchRequest(...)
// (recon/PAPYRUS-CONTRACT.md §2; recon/strings.txt §4). The action vocabulary
// the script accepts, parsed before the first '|', is:
//   catalogRequest, stageNext, stagePrev, sceneNext, scenePrev, scene,
//   dressMale, undressMale, dressFemale, undressFemale, speed, pause,
//   resume, end, orgasm, freecam, mute
// The C++ side does not need to understand them: it forwards the payload
// verbatim as the mod-event string argument. Only the two UI-local actions are
// handled in-module: `catalogRequest` triggers the catalogue publish and
// `slppCatalogRetry` asks for a smaller-slice republish.
namespace ActionDispatch
{
	// "slppAction": parse `action|arg`, forward to Papyrus as SLPPPrism_Action.
	void HandleAction(const char* a_payload);

	// "slppSearchRequest": ask the controller to open/defer the text-entry
	// search (mod event SLPPPrism_SearchRequest).
	void HandleSearchRequest(const char* a_currentText);

	// "slppCollapsed": `a_arg` is "1" for collapsed, "0" for expanded. `a_source`
	// is the origin label used in the original's `Collapse state -> {} (via {})`.
	void SetCollapsed(bool a_collapsed, const char* a_source);

	// "slppReady": the HTML run-loop handshake (recon/controller-0.6.1.html).
	void HandleReady(const char* a_arg);

	// "slppLog": UI console line relayed into the plugin log.
	void HandleLog(const char* a_message);

	// "slppCatalogRetry": the UI received fewer rows than `total`; republish with
	// a smaller per-invoke slice (recon/strings.txt §5, "Catalog incomplete on UI
	// side, republishing with smaller slices (attempt {})").
	void HandleCatalogRetry(const char* a_arg);

	// Shared mod-event sender: `SKSE::GetModCallbackEventSource()->SendEvent`.
	void SendModEvent(const char* a_eventName, const char* a_strArg, float a_numArg);
}  // namespace ActionDispatch