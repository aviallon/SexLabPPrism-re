#pragma once

// MenuVisibilitySink — the plugin's `RE::BSTEventSink<RE::MenuOpenCloseEvent>`.
//
// Recovered from 0x18001f3c0 (recon/NATIVES-RECOVERED.md §4.3): keeps the
// vanilla HUD and the developer console consistent with the Prism scene state
// as those menus open and close. It only ever touches the two vanilla menus, so
// a third-party menu such as UIExtensions' "CustomMenu" is never hidden or
// removed by this sink.
//
// The original class is `?AVMenuVisibilitySink@?A0xbb2e73b6@@`, TOP-LEVEL in the
// unity TU's anonymous namespace, and its ProcessEvent bakes the __FUNCSIG__
// ``anonymous-namespace'::MenuVisibilitySink::ProcessEvent``. The class lives in
// MenuVisibilitySink.cpp's global anonymous namespace; only the registration
// entry point is exposed here.
namespace PrismSinks
{
	// Register the singleton sink on RE::UI's MenuOpenCloseEvent source.
	void RegisterMenuVisibility();
}  // namespace PrismSinks
