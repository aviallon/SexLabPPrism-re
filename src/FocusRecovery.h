#pragma once

#include <atomic>
#include <cstdint>

// FocusRecovery — the free-camera / Prisma-focus reassertion state machine.
//
// Recovered from recon/BINARY-RECON.md §1.4 (src\main.cpp) and the original's
// log strings at recon/strings.txt:191-198. The exact API recovered from the
// log call sites is:
//
//   void FocusRecovery::Begin(IVPrismaUI1*, unsigned __int64,
//                             const std::atomic<bool>*, const std::atomic<bool>*)  0x1800130e0
//   void FocusRecovery::Cancel(void)                                                 0x180013290
//   FocusRecovery::`anonymous-namespace'::CheckUnfocus(unsigned __int64, int)        0x180013408
//   FocusRecovery::`anonymous-namespace'::Finalize(unsigned __int64)                 0x180013760
//   FocusRecovery::`anonymous-namespace'::CloseOwnedConsole(void)                    0x1800136b0
//   FocusRecovery::`anonymous-namespace'::StartConsolePulse(unsigned __int64)        0x180014360 / 0x180014428
//   FocusRecovery::`anonymous-namespace'::VerifyCleanup(unsigned __int64, int, bool) 0x1800145c5 / 0x1800146e0
//
// The helper entry points live in an anonymous namespace *inside* FocusRecovery
// exactly as in the original, which is what makes the compiler bake the
// `FocusRecovery::`anonymous-namespace'::Name` __FUNCSIG__ at their log sites
// (the identity anchor the matcher recovers). They are therefore not declared
// here; only Begin/Cancel/Run are public.
namespace FocusRecovery
{
	// Arm a new recovery generation. Called from the F4 hotkey when Prisma is
	// being released back to the camera. The interface/view handles and the two
	// atomic state flags are the original's arguments; nullptr is accepted
	// because the current SceneState helpers do not expose the underlying
	// atomics (marked gap).
	void Begin(void* a_iface, std::uint64_t a_view, const std::atomic<bool>* a_sceneActive, const std::atomic<bool>* a_uiMode);

	// Disarm the current generation. Called on F4 back into UI mode, and by the
	// scene-state transition in SceneState::Publish (the natives' only call).
	void Cancel();

	[[nodiscard]] bool IsArmed();

	// One step of the retry loop, driven from the F4 path's task. Returns true
	// while the recovery is still in progress.
	bool Run();

	inline constexpr int kMaxUnfocusAttempts = 5;
}  // namespace FocusRecovery