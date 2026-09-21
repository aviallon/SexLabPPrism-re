#pragma once

// InputSink — the plugin's `RE::BSTEventSink<RE::InputEvent*>`.
//
// Recovered from 0x18001f0b0 (recon/NATIVES-RECOVERED.md §4.2): while a scene
// is active, suppress the developer-console scancode 0x29 (`~`); on F4 (0x3E)
// toggle UI/camera mode and queue a presentation refresh; on F3 (0x3D) toggle
// the interface hidden/restored and re-apply the presentation. F3/F4 are
// ignored while the modal search box is open.
//
// In the original the sink class itself is `?AVInputSink@?A0xbb2e73b6@@`, a
// TOP-LEVEL class in the unity TU's anonymous namespace (not a `Sink` nested
// inside a named `InputSink` namespace). Its ProcessEvent therefore bakes the
// __FUNCSIG__ ``anonymous-namespace'::InputSink::ProcessEvent`. The class lives
// in InputSink.cpp's global anonymous namespace; only the registration entry
// point is exposed here.
namespace PrismSinks
{
	// Register the singleton sink on the game's input event source
	// (BSInputDeviceManager, a BSTEventSource<InputEvent*>).
	void RegisterInput();
}  // namespace PrismSinks
