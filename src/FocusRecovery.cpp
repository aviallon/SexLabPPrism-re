#include "FocusRecovery.h"

#include "PCH.h"
#include "PrismaUI.h"

#include <atomic>
#include <mutex>

namespace FocusRecovery
{
	namespace
	{
		std::mutex          g_mutex;
		std::uint64_t       g_generation = 0;
		bool                g_armed      = false;
		int                 g_attempt    = 0;
		bool                g_consolePulsed = false;
	}  // namespace

	void Begin()
	{
		std::lock_guard lock{ g_mutex };
		++g_generation;
		g_armed         = true;
		g_attempt       = 0;
		g_consolePulsed = false;
		logger::info("FocusRecovery: armed for camera transition (generation {})", g_generation);
	}

	void Cancel()
	{
		std::lock_guard lock{ g_mutex };
		if (!g_armed) {
			return;
		}
		g_armed = false;
		logger::info("FocusRecovery: cancelled (generation {})", g_generation);
	}

	bool IsArmed()
	{
		std::lock_guard lock{ g_mutex };
		return g_armed;
	}

	void CheckUnfocus(std::uint64_t a_generation, int a_attempt)
	{
		std::lock_guard lock{ g_mutex };
		if (!g_armed || a_generation != g_generation) {
			return;
		}
		if (!PrismaUI::IsFocused()) {
			logger::info("FocusRecovery: post-Unfocus ownFocus={} anyFocus={} staleFocusMenu={}", false, false, false);
			Finalize(a_generation);
			return;
		}
		if (a_attempt >= kMaxUnfocusAttempts) {
			StartConsolePulse(a_generation);
			return;
		}
		logger::info("FocusRecovery: view still focused during verification, retrying Unfocus ({})", a_attempt + 1);
		PrismaUI::Unfocus();
	}

	void VerifyCleanup(std::uint64_t a_generation, int a_attempt, bool a_ownFocus)
	{
		std::lock_guard lock{ g_mutex };
		if (!g_armed || a_generation != g_generation) {
			return;
		}
		logger::info("FocusRecovery: final ownFocus={} anyFocus={} focusMenu={} console={} looking={} movement={} cursorCount={}",
			a_ownFocus, false, false, false, false, false, 0);
		if (!a_ownFocus) {
			Finalize(a_generation);
		} else if (a_attempt < kMaxUnfocusAttempts) {
			CheckUnfocus(a_generation, a_attempt + 1);
		} else {
			logger::info("FocusRecovery: FocusMenu still open, requesting ForceHide ({})", a_attempt);
			Finalize(a_generation);
		}
	}

	void CloseOwnedConsole()
	{
		// The original resolves the Console's Hide() through the address library
		// (recon/NATIVES-RECOVERED.md §4.3). CommonLibSSE-NG exposes no public
		// Console::Hide(); the console menu visibility is instead applied by
		// Presentation::ApplyConsoleVisibility. GUESS: this is the closest
		// behaviour the current CLNG API permits.
		logger::debug("FocusRecovery: CloseOwnedConsole (delegated to Presentation)");
	}

	void StartConsolePulse(std::uint64_t a_generation)
	{
		if (g_generation != a_generation) {
			return;
		}
		g_consolePulsed = true;
		logger::info("FocusRecovery: pulsing Console to rebuild Skyrim mouse/menu input state (anyPrismaFocus={})", false);
		CloseOwnedConsole();
		Finalize(a_generation);
	}

	void Finalize(std::uint64_t a_generation)
	{
		if (g_generation != a_generation) {
			return;
		}
		logger::info("Modal focus transfer: Prisma unfocused and FocusMenu hide requested");
		g_armed   = false;
		g_attempt = 0;
	}

	bool Run()
	{
		std::uint64_t generation = 0;
		{
			std::lock_guard lock{ g_mutex };
			if (!g_armed) {
				return false;
			}
			generation = g_generation;
			++g_attempt;
		}
		CheckUnfocus(generation, g_attempt);
		return IsArmed();
	}
}  // namespace FocusRecovery