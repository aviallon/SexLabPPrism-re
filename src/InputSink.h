#pragma once

// InputSink — the plugin's `RE::BSTEventSink<RE::InputEvent*>`.
//
// Recovered from 0x18001f0b0 (recon/NATIVES-RECOVERED.md §4.2): while a scene
// is active, suppress the developer-console scancode 0x29 (`~`); on F4 (0x3E)
// toggle UI/camera mode and queue a presentation refresh; on F3 (0x3D) toggle
// the interface hidden/restored and re-apply the presentation. F3/F4 are
// ignored while the modal search box is open.
namespace InputSink
{
	// Register the singleton sink on the game's input event source
	// (BSInputDeviceManager, a BSTEventSource<InputEvent*>).
	void Register();
}  // namespace InputSink