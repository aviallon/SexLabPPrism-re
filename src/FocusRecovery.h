#pragma once

#include <cstdint>

// FocusRecovery — the free-camera / Prisma-focus reassertion state machine.
//
// Recovered from recon/BINARY-RECON.md §1.4 (src\FocusRecovery.cpp):
//   FocusRecovery::Begin(IVPrismaUI1*, view, bool* sceneActive, bool* uiMode)  0x180013122
//   FocusRecovery::Cancel()                                                    0x180013290
//   FocusRecovery::anon::CheckUnfocus(uint64 generation, int attempt)          0x180013408
//   FocusRecovery::anon::Finalize(uint64 generation)                           0x180013760
//   FocusRecovery::anon::CloseOwnedConsole()                                   0x1800136b0
//   FocusRecovery::anon::StartConsolePulse(uint64 generation)                  0x180014360
//   FocusRecovery::anon::VerifyCleanup(uint64 generation, int, bool)           0x1800145c5
//
// The original arms a generation counter on the F4 transition, then retries
// "wait for Prisma Unfocus" up to five times before pulsing the Console to
// rebuild Skyrim's mouse/menu input state (log strings in recon/strings.txt §4
// and §5). The exact PrismaUI focus slots are external and unrecovered, so the
// retry loop uses PrismaUI::IsFocused()/Unfocus() (which are the single
// documented indirection point) and the state machine is exercised by Run().
namespace FocusRecovery
{
	// Arm a new recovery generation. Called from the F4 hotkey when Prisma is
	// being released back to the camera (recon/NATIVES-RECOVERED.md §4.2).
	void Begin();

	// Disarm the current generation. Called on F4 back into UI mode, and by the
	// scene-state transition in SceneState::Publish (the natives' only call).
	void Cancel();

	[[nodiscard]] bool IsArmed();

	// One step of the retry loop: waits for the Prisma view to unfocus, retries
	// up to kMaxUnfocusAttempts, then verifies cleanup and finalizes. Returns
	// true while the recovery is still in progress.
	bool Run();

	// The recovered helper entry points, kept separate so the structure matches
	// the original even though Run() is what drives them.
	void CheckUnfocus(std::uint64_t a_generation, int a_attempt);
	void Finalize(std::uint64_t a_generation);
	void VerifyCleanup(std::uint64_t a_generation, int a_attempt, bool a_ownFocus);
	void CloseOwnedConsole();
	void StartConsolePulse(std::uint64_t a_generation);

	inline constexpr int kMaxUnfocusAttempts = 5;
}  // namespace FocusRecovery