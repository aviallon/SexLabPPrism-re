#include "FocusRecovery.h"

#include "PCH.h"
#include "Presentation.h"
#include "PrismaUI.h"
#include "SceneState.h"

#include <atomic>
#include <mutex>
#include <string_view>

namespace FocusRecovery
{
	// The whole state machine lives in an anonymous namespace inside
	// FocusRecovery, matching the original's src\main.cpp layout so the
	// `FocusRecovery::`anonymous-namespace'::Name` __FUNCSIG__ strings baked by
	// the log calls are recoverable by tools/parity_names.py.
	namespace
	{
		// DAT_18009c1c8-ish: the armed generation. Begin() bumps it, Cancel()
		// drops it, and every helper validates its generation against it; a
		// stale task therefore cannot act on a newer transition.
		std::mutex          g_mutex;
		std::uint64_t       g_generation  = 0;
		bool                g_armed       = false;
		int                 g_attempt     = 0;
		bool                g_consolePulsed = false;
		void*               g_iface       = nullptr;
		std::uint64_t       g_view        = 0;
		const std::atomic<bool>* g_sceneActive = nullptr;
		const std::atomic<bool>* g_uiMode      = nullptr;

		void VerifyCleanup(std::uint64_t a_generation, int a_attempt, bool a_ownFocus);
		void StartConsolePulse(std::uint64_t a_generation);
		void CloseConsolePulse(std::uint64_t a_generation);

		[[nodiscard]] bool IsCurrent(std::uint64_t a_generation)
		{
			return g_armed && a_generation == g_generation;
		}

		// FUN_180013a10 in the original: "is the recovery console already up?".
		// Implemented against the vanilla Console menu (self-contained; the exact
		// original predicate is unresolved).
		[[nodiscard]] bool IsConsoleOpen()
		{
			auto* const ui = RE::UI::GetSingleton();
			return ui != nullptr && ui->GetMenu(RE::Console::MENU_NAME) != nullptr;
		}

		// The "PrismaUI_FocusMenu" managed menu (PTR_s_PrismaUI_FocusMenu_180056a00).
		[[nodiscard]] bool IsFocusMenuOpen()
		{
			auto* const ui = RE::UI::GetSingleton();
			return ui != nullptr && ui->GetMenu(std::string_view{ "PrismaUI_FocusMenu" }) != nullptr;
		}

		// 0x1800136b0: clears the recovery-console latch (DAT_18009c0b0) and, if
		// it was set, logs and close/hides the Console (FUN_180014140(&"Console",3)).
		void CloseOwnedConsole()
		{
			const bool wasOwned = g_consolePulsed;
			g_consolePulsed     = false;
			if (wasOwned) {
				logger::info("FocusRecovery: closing recovery console");
				Presentation::ApplyConsoleVisibility(true);
			}
		}

		// 0x180016d20 (17 insns): FocusRecovery::`anonymous-namespace'::CloseConsolePulse.
		// The original closes the recovery console immediately, then schedules a
		// follow-up through the task interface with a 0x78 (120 ms) delay
		// (FUN_180013f20(task, 0x78, fn)). The captured generation is carried into
		// the delayed lambda; the delay argument is not reproduced here because
		// the SKSE TaskInterface wrapper exposes no delayed AddTask (marked gap).
		void CloseConsolePulse(std::uint64_t a_generation)
		{
			CloseOwnedConsole();
			auto* const task = SKSE::GetTaskInterface();
			if (!task) {
				return;
			}
			task->AddTask([a_generation]() { VerifyCleanup(a_generation, kMaxUnfocusAttempts, false); });
		}

		// 0x180013760: re-read every focus slot and log the seven-slot final line;
		// when the generation is no longer valid it just releases the console.
		void Finalize(std::uint64_t a_generation)
		{
			if (!IsCurrent(a_generation)) {
				CloseOwnedConsole();
				return;
			}
			const bool ownFocus      = PrismaUI::IsFocused();
			const bool anyFocus      = PrismaUI::IsFocused();  // slot 0xa0 not exposed
			const bool focusMenu     = IsFocusMenuOpen();
			const bool consoleOpen   = IsConsoleOpen();
			const bool looking       = false;  // FUN_180037490-backed; not exposed
			const bool movement      = false;  // FUN_1800374f0-backed; not exposed
			const std::uint32_t cursorCount = 0;
			logger::info("FocusRecovery: final ownFocus={} anyFocus={} focusMenu={} console={} looking={} movement={} cursorCount={}",
				ownFocus, anyFocus, focusMenu, consoleOpen, looking, movement, cursorCount);
			g_armed   = false;
			g_attempt = 0;
		}

		// 0x180014360 / 0x180014428: the Console "pulse" fallback when focus
		// survives verification. Refuses to run while the Console is already open
		// or a pulse is in flight, then force-hides the Console and closes it.
		void StartConsolePulse(std::uint64_t a_generation)
		{
			if (!IsCurrent(a_generation)) {
				return;
			}
			if (IsConsoleOpen()) {
				logger::info("FocusRecovery: console fallback skipped because Console was already open");
				return;
			}
			const bool wasIdle = !g_consolePulsed;
			g_consolePulsed    = true;
			if (!wasIdle) {
				logger::info("FocusRecovery: console pulse already in progress");
				return;
			}
			logger::info("FocusRecovery: pulsing Console to rebuild Skyrim mouse/menu input state (anyPrismaFocus={})", PrismaUI::IsFocused());
			// 0x180016d20: the original closes the console and schedules the
			// follow-up with a 0xb4/0x78 ms delay via FUN_180013f20.
			CloseConsolePulse(a_generation);
		}

		// 0x180013408: wait for the Prisma view to release focus, retrying the
		// Unfocus call up to kMaxUnfocusAttempts. "waiting for Prisma Unfocus
		// ({}/5)" is the recovered counter message.
		void CheckUnfocus(std::uint64_t a_generation, int a_attempt)
		{
			if (!IsCurrent(a_generation)) {
				return;
			}
			const bool ownFocus = PrismaUI::IsFocused();
			const bool anyFocus      = ownFocus;
			const bool staleFocusMenu = false;
			logger::info("FocusRecovery: post-Unfocus ownFocus={} anyFocus={} staleFocusMenu={}", ownFocus, anyFocus, staleFocusMenu);
			if (!ownFocus) {
				VerifyCleanup(a_generation, a_attempt, false);
				return;
			}
			if (a_attempt >= kMaxUnfocusAttempts) {
				logger::info("FocusRecovery: foreign Prisma focus detected after F4; requesting Console yield");
				StartConsolePulse(a_generation);
				return;
			}
			logger::info("FocusRecovery: waiting for Prisma Unfocus ({}/5)", a_attempt + 1);
			PrismaUI::Unfocus();
		}

		// 0x1800145c5 / 0x1800146e0: the verification stage. `a_ownFocus` is the
		// pre-Unfocus ownership flag; the current slots decide whether to retry
		// (attempt < 3), force-hide the FocusMenu, finalize, or pulse the Console.
		void VerifyCleanup(std::uint64_t a_generation, int a_attempt, bool a_ownFocus)
		{
			if (!IsCurrent(a_generation)) {
				return;
			}
			const bool ownFocus  = PrismaUI::IsFocused();
			const bool anyFocus  = ownFocus;  // slot 0xa0 not exposed
			const bool focusMenu = IsFocusMenuOpen();
			if (!ownFocus) {
				if (anyFocus) {
					logger::info("FocusRecovery: another Prisma view acquired focus; using Console yield");
					StartConsolePulse(a_generation);
					return;
				}
			} else if (a_attempt < 3) {
				logger::info("FocusRecovery: view still focused during verification, retrying Unfocus ({})", a_attempt + 1);
				PrismaUI::Unfocus();
			}
			if (focusMenu) {
				logger::info("FocusRecovery: FocusMenu still open, requesting ForceHide ({})", a_attempt + 1);
				if (auto* const ui = RE::UI::GetSingleton()) {
					if (const auto m = ui->GetMenu(std::string_view{ "PrismaUI_FocusMenu" }); m && m->uiMovie) {
						m->uiMovie->SetVisible(false);
					}
				}
			}
			if ((ownFocus || focusMenu) && a_attempt < 3) {
				// Original re-arms VerifyCleanup through the task interface with a
				// 0x50 ms delay (FUN_180013f20); the delay is not reproduced
				// (marked gap).
				return;
			}
			if (!a_ownFocus && !ownFocus && !focusMenu) {
				Finalize(a_generation);
				return;
			}
			StartConsolePulse(a_generation);
		}
	}  // namespace

	__declspec(noinline) void Begin(void* a_iface, std::uint64_t a_view, const std::atomic<bool>* a_sceneActive, const std::atomic<bool>* a_uiMode)
	{
		std::lock_guard lock{ g_mutex };
		++g_generation;
		g_armed         = true;
		g_attempt       = 0;
		g_consolePulsed = false;
		g_iface         = a_iface;
		g_view          = a_view;
		g_sceneActive   = a_sceneActive;
		g_uiMode        = a_uiMode;
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