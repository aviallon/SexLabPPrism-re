#pragma once

// MenuVisibilitySink — the plugin's `RE::BSTEventSink<RE::MenuOpenCloseEvent>`.
//
// Recovered from 0x18001f3c0 (recon/NATIVES-RECOVERED.md §4.3): keeps the
// vanilla HUD and the developer console consistent with the Prism scene state
// as those menus open and close. It only ever touches the two vanilla menus, so
// a third-party menu such as UIExtensions' "CustomMenu" is never hidden or
// removed by this sink.
namespace MenuVisibilitySink
{
	// Register the singleton sink on RE::UI's MenuOpenCloseEvent source.
	void Register();
}  // namespace MenuVisibilitySink