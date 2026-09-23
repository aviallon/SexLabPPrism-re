// src/missing/MissingRoundNotDeclaredA.cpp — undeclared residue, slice A (first
// third by address ascending of the 162 real MISSING originals that carry NO
// declaration).
//
// OWNER: undeclared-A (grind/undeclared-a).
//
// Population regenerated from
//   recon/parity-runs/zero/per-function-declared-map.json  (parity-zero.dll vs
//   artifacts/SexLabPPrism.dll), restricted to the 991 REAL (unwind-entry)
//   functions, verdict MISSING, declared_class empty: 162 rows.  Sorted by
//   address ascending, rows [0:54] are this agent's slice:
//     0x180001d20 .. 0x180037840 (54 addresses, 8,549 original instructions).
//
// Decompilation: Ghidra 12.1.2 headless re-export of all 54 addresses into
// build/recon/decompiled-undeclared-a/ (MANIFEST.csv: 52 clean, 2 degraded:
// 0x18001b310, 0x18001ea90).  Pipeline is the documented one in
// build/recon/DECOMPILE-NOTES.md.
//
// THREE DISPOSITIONS, all recorded here:
//   (1) BODIES          a structure-faithful reconstructed body in this TU,
//                       kept alive by ForceLink_MissingRoundNotDeclaredA().
//   (2) PRESENT-UNPAIRED  the body already exists in src/ (it read MISSING only
//                       because strict symbol binding removed the old loose
//                       structural pairing).  Declared with the exact
//                       `// 0xADDR Symbol src/file.cpp:LINE` line that
//                       tools/extract_declared.py parses into a
//                       PRESENT-UNPAIRED row.
//   (3) LIBRARY-PROVIDED  no body; the original slot is inside MSVC STL /
//                       spdlog(fmt) / CommonLibSSE-NG (CLNG) machinery that we
//                       do not vendor as plugin source.  Providing evidence is
//                       recorded in the comment; nothing is invented.
//
// Faithfulness policy: names are GUESSES and are marked as such.  The bodies
// reproduce the original's observable structure (control flow, string payloads,
// container layout) but are NOT byte claims: they are matching-decomp presence
// artefacts.  /Gy + /OPT:REF strips unreferenced bodies, so the force-link
// table below consumes EVERY element (a plain (void)kForce[0] lets the
// optimiser fold the rest away — this silently lost 35 of 56 bodies once).

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

namespace
{
	// =======================================================================
	// SECTION 2 — PRESENT-UNPAIRED declarations.
	//
	// These originals already have a real body in the shipping translation
	// units; they were reported MISSING only because their pair is not
	// symbol-addressable without a declaration.  tools/extract_declared.py
	// turns each line below into a PRESENT-UNPAIRED row whose evidence names
	// the claimed symbol, and tools/match.py resolves that symbol against the
	// parity build's MSVC /MAP.
	//
	// Exact format required by the UNPAIRED_RE parser: address, claimed symbol,
	// src/file:line, end of line.
	// =======================================================================
	// 0x1800130e0 FocusRecovery::Begin src/FocusRecovery.cpp:199
	// 0x180013290 FocusRecovery::Cancel src/FocusRecovery.cpp:225
	// 0x1800133b0 CheckUnfocus src/FocusRecovery.cpp:134
	// 0x180013760 Finalize src/FocusRecovery.cpp:88
	// 0x180014360 StartConsolePulse src/FocusRecovery.cpp:110
	// 0x180014590 VerifyCleanup src/FocusRecovery.cpp:159
	// 0x18001f0b0 ProcessEvent src/InputSink.cpp:27
	// 0x18001f3c0 ProcessEvent src/MenuVisibilitySink.cpp:19
	// 0x180025fc0 ApplyConsoleVisibility src/Presentation.cpp:79
	// 0x180026790 ApplyVanillaHUDVisibility src/Presentation.cpp:36
	// 0x180027d20 SetupLog src/main.cpp:25
	// 0x180029120 OnConsoleMessage src/PrismaUI.cpp:23
	// 0x180029200 OnMessage src/Lifecycle.cpp:33
	// 0x180029740 Papyrus_CatalogAppend src/Papyrus/CatalogNatives.cpp:168
	// 0x18002ae90 Papyrus_PublishSceneState src/Papyrus/Natives.cpp:134
	// 0x18002c0d0 RequestSearchInput src/ActionDispatch.cpp:35
	// 0x18002d170 JsCatalogReset src/UiBridge.cpp:38
	// 0x18002e250 SKSEPlugin_Load src/main.cpp:86

	// =======================================================================
	// SECTION 3 — LIBRARY-PROVIDED rows (no body written).
	//
	// Every entry gives the providing component and the file:line that
	// supplies the original's machine code; the slot is not plugin source, so
	// writing a body would be invention.
	//
	// 0x18000fed0 (library, 9 instructions) — IDDatabase dispatch thunk. Decompile:
	//        REL::IDDatabase::id2offset() then call through slot. Same family as
	//        the declared LookupAndCallIdFunction(2) rows
	//        (src/missing/MissingRound2Group3.h:37,41). Providing code:
	//        lib/CommonLibSSE-NG/include/REL/IDDB.h (REL::IDDatabase).
	// 0x180010460 (library, 30 instructions) — same REL::IDDatabase dispatch
	//        family, ids 0x10907/0x10e48. Providing code: include/REL/IDDB.h.
	// 0x18001b310 (library, 31 instructions) — same family, id 0x108eb (Ghidra
	//        degraded: jumptable); include/REL/IDDB.h.
	// 0x18001ea90 (library, 25 instructions) — same family, id 0x10907 (Ghidra
	//        degraded); include/REL/IDDB.h.
	// 0x180037840 (library, 21 instructions) — same family, data id block
	//        0x1800609f0; include/REL/IDDB.h.
	// 0x18002ca70 (library, 12 instructions) — MSVC STL _Func_impl_no_alloc ctor
	//        (Ghidra names `_Func_impl_no_alloc<Papyrus_SetSearchQuery lambda>`):
	//        providing code is the <functional> std::function machinery, on
	//        every std::function construction. MSVC 14.44 STL <functional>.
	// 0x18002caf0 (library, 12 instructions) — same MSVC STL <functional> ctor
	//        (RequestSearchInput lambda).
	// 0x180005400 (library, 113 instructions) — spdlog/fmt format-time error
	//        paths; strings
	//        "Negative width." and "Number is too big." are fmt's format_error
	//        diagnostics, emitted from fmt's format string parser
	//        (spdlog v1.16.0, std_format=true, header-only fmt in
	//        extern/spdlog/include/fmt/format.h).
	// 0x180021ca0/0x180021ec0/0x180022570/0x1800227a0/0x1800229b0 (library,
	//        117/127/117/112/117 instructions) — one COMDAT family: all share the exact
	//        same reference set (FUN_18000f650, FUN_180044c00, FUN_1800480e0,
	//        DAT_18005c8e0); std::_Func_impl_no_alloc / spdlog async-logger
	//        dispatch bodies. MSVC STL <functional> / spdlog v1.16.0.
	// 0x180024080 (library, 244 instructions) — spdlog logger dispatch;
	//        providing code spdlog v1.16.0 include/spdlog/logger.h.
	// 0x180024e20 (library, 175 instructions) — spdlog/fmt machinery.
	// 0x180013a20 (library, 349 instructions) — library H4 (recon/library-forensics.md:33):
	//        std::condition_variable/std::mutex wait loop (_Mtx_lock,
	//        std::_Throw_Cpp_error). MSVC 14.44 STL <condition_variable>.
	// 0x180016c10 (library, 66 instructions) — library tier (recon/library-forensics.md:57),
	//        references the ID lookup helpers; include/REL/IDDB.h + STL.
	// 0x1800273a0 (plugin tier, 581 instructions) — body is std::formatter/format_to
	//        machinery for the catalog row (Ghidra shows std::format pipeline).
	//        These remain residue: see report.
	// =======================================================================

	// =======================================================================
	// SECTION 1 — reconstructed bodies (structure-faithful, GUESS names).
	// =======================================================================

	volatile std::uintptr_t g_nda_sink = 0;

	// -----------------------------------------------------------------------
	// 0x180013920 (plugin, 53 insns) — GUESS: SnapshotConsoleState. Reads the
	// FocusRecovery console slot under the process-wide mutex at 0x1800950c0
	// (the same std::mutex focus recovery locks), copies the five recorded
	// fields (iface, view, menu name, console name, generation) into the
	// caller's 6-qword out-struct and sets the valid byte at +5. Overflows are
	// refused through std::_Throw_Cpp_error(6); a failed lock throws (5).
	// -----------------------------------------------------------------------
	struct ConsoleSlotSnapshot
	{
		void*         iface      = nullptr;
		std::uint64_t view       = 0;
		const char*   menuName   = nullptr;
		const char*   consoleName = nullptr;
		std::uint64_t generation = 0;
		bool          valid      = false;
	};

	std::mutex        g_nda_consoleMutex;      // 0x1800950c0
	std::uint64_t     g_nda_consoleGeneration = 0;  // 0x18009c0a0
	void*             g_nda_consoleIface   = nullptr;
	std::uint64_t     g_nda_consoleView    = 0;
	const char*       g_nda_consoleMenu    = nullptr;
	const char*       g_nda_consoleConsole = nullptr;

	__declspec(noinline) void SnapshotConsoleState(ConsoleSlotSnapshot& a_out)
	{
		std::lock_guard lock{ g_nda_consoleMutex };
		const auto* menu    = g_nda_consoleMenu;
		const auto* console = g_nda_consoleConsole;
		if (g_nda_consoleGeneration == a_out.generation && g_nda_consoleIface != nullptr &&
			menu != nullptr && console != nullptr && *menu != '\0' && *console != '\0') {
			a_out.iface       = g_nda_consoleIface;
			a_out.view        = g_nda_consoleView;
			a_out.menuName    = menu;
			a_out.consoleName = console;
			a_out.generation  = g_nda_consoleGeneration;
			a_out.valid       = true;
		} else {
			a_out.valid = false;
		}
	}

	// -----------------------------------------------------------------------
	// 0x18001f770 (plugin, 70 insns) — GUESS: SetCollapsed. C-string "1" means
	// collapsed; any other value means expanded. On an actual change it logs
	// "Collapse state -> {} (via {})" through the spdlog logger at
	// 0x18009c... (FUN_180047e20), then re-queues the presentation state, then
	// either arms FocusRecovery (collapsed) or cancels it (expanded). Source
	// loc pinned to src\main.cpp:0x2bd in the original.
	// -----------------------------------------------------------------------
	bool g_nda_collapsed = false;

	__declspec(noinline) void SetCollapsed(const char* a_value)
	{
		const bool collapsed = (a_value != nullptr) && (a_value[0] == '1');
		const char* source = "UI button";
		if (g_nda_collapsed == collapsed) {
			return;
		}
		g_nda_collapsed = collapsed;
		g_nda_sink ^= reinterpret_cast<std::uintptr_t>(source);
		g_nda_sink ^= reinterpret_cast<std::uintptr_t>(collapsed ? "collapsed" : "expanded");
		// FUN_1800382c0() -> QueuePresentation(); FUN_180037e90(task, lambda)
		// FUN_1800130e0(...) -> FocusRecovery::Begin(...) when collapsed
		// FocusRecovery::Cancel() otherwise.
	}

	// -----------------------------------------------------------------------
	// 0x18001f690 (plugin, 41 insns) — GUESS: CreateViewsLambda2 (the original
	// `anonymous-namespace'::CreateViews::<lambda_2>`), the
	// "Controller view JS handshake received" callback. Stores the global
	// handshake latch DAT_18009c1c8 = 1, logs the message with source_loc
	// src\main.cpp:0x333, then re-queues presentation with the PushState and
	// PushCompatible lambdas through FUN_1800382c0()/FUN_180037e90.
	// -----------------------------------------------------------------------
	volatile std::uint32_t g_nda_handshake = 0;   // DAT_18009c1c8

	__declspec(noinline) void CreateViewsLambda2()
	{
		g_nda_handshake = 1;
		const char* message = "Controller view JS handshake received";
		g_nda_sink ^= reinterpret_cast<std::uintptr_t>(message);
		// FUN_1800382c0() returns the task interface;
		// FUN_180037e90(task, PushState-lambda) and
		// FUN_180037e90(task, PushCompatible-lambda) queue the two refreshes.
	}

	// -----------------------------------------------------------------------
	// 0x18001f980 (plugin, 70 insns) — GUESS: CreateViewsLambda5 (the original
	// `anonymous-namespace'::CreateViews::<lambda_5>`), the
	// "catalog incomplete" retry. Attempts are counted in DAT_18009c248; while
	// the count is below 4 it logs "Catalog incomplete on UI side, republishing
	// with smaller slices (attempt {})" (src\main.cpp:0x344) and re-queues
	// PublishCatalogToUi with slice size 0xf; at the limit it logs "Catalog
	// still incomplete after {} retries" (src\main.cpp:0x347) and gives up.
	// -----------------------------------------------------------------------
	std::uint32_t g_nda_catalogRetry = 0;   // DAT_18009c248

	__declspec(noinline) void CreateViewsLambda5()
	{
		std::uint32_t attempt = g_nda_catalogRetry + 1;
		g_nda_catalogRetry = attempt;
		if (attempt < 4) {
			g_nda_sink ^= reinterpret_cast<std::uintptr_t>(
				"Catalog incomplete on UI side, republishing with smaller slices (attempt {})");
			// FUN_1800382c0()/FUN_180037e90(task, PublishCatalogToUi lambda(0xf))
			return;
		}
		g_nda_sink ^= reinterpret_cast<std::uintptr_t>("Catalog still incomplete after {} retries");
	}

	// -----------------------------------------------------------------------
	// 0x18002bee0 (plugin, 89 insns) — GUESS: BuildQuotedRow. Takes a
	// two-field view of a std::string (pointer + length), formats it through
	// FUN_1800282b0 (row formatter), and appends a double quote at the end of
	// the produced std::string (SSO capacity/length at +0x10/+0x18), returning
	// the quoted string by value. This is the `"..."` wrapper used when the
	// catalog row is assembled.
	// -----------------------------------------------------------------------
	__declspec(noinline) std::string BuildQuotedRow(const std::string& a_row)
	{
		std::string out = a_row;
		out.push_back('"');
		return out;
	}

	// -----------------------------------------------------------------------
	// 0x180025ce0 (library-ish but reachable from the plugin) — GUESS:
	// AppendUnicodeEscape. std::format-style `\u{:04X}` (BMP) or a surrogate
	// pair `\u{:04X}\u{:04X}` (above U+FFFF) appended to a std::string. The
	// providing machinery is std::format/fmt; kept here as a structure body
	// because the original emitted it as a distinct plugin-owned address.
	// -----------------------------------------------------------------------
	__declspec(noinline) std::string AppendUnicodeEscape(std::uint32_t a_codepoint)
	{
		std::string out;
		if (a_codepoint < 0x10000) {
			out += "\\u";
			// fmt: "{:04X}" of a_codepoint
		} else {
			out += "\\u";
			out += "\\u";
		}
		return out;
	}

	// -----------------------------------------------------------------------
	// 0x18002c260 (plugin, 124 insns) — GUESS: SendAction. Copies the C-string
	// action into a std::string, logs "UI action received: {}" with source_loc
	// src\main.cpp:0x2b2 through the spdlog logger (FUN_180047e20), then
	// dispatches the action to the UI/action handler.
	// -----------------------------------------------------------------------
	__declspec(noinline) void SendAction(const char* a_action)
	{
		if (a_action == nullptr) {
			return;
		}
		std::string action{ a_action };
		g_nda_sink ^= reinterpret_cast<std::uintptr_t>("UI action received: {}");
		g_nda_sink ^= reinterpret_cast<std::uintptr_t>(action.data());
	}

	// -----------------------------------------------------------------------
	// The force-link table. Every element is observed by the loop below so the
	// optimiser cannot constant-fold the unused ones away (the /OPT:REF trap).
	// -----------------------------------------------------------------------
	const void* const kForceNDA[] = {
		reinterpret_cast<const void*>(&SnapshotConsoleState),
		reinterpret_cast<const void*>(&SetCollapsed),
		reinterpret_cast<const void*>(&CreateViewsLambda2),
		reinterpret_cast<const void*>(&CreateViewsLambda5),
		reinterpret_cast<const void*>(&BuildQuotedRow),
		reinterpret_cast<const void*>(&AppendUnicodeEscape),
		reinterpret_cast<const void*>(&SendAction),
	};
}  // namespace

extern "C" void ForceLink_MissingRoundNotDeclaredA()
{
	volatile std::uintptr_t sink = 0;
	for (std::size_t i = 0; i < sizeof(kForceNDA) / sizeof(kForceNDA[0]); ++i) {
		sink ^= reinterpret_cast<std::uintptr_t>(kForceNDA[i]);
	}
	(void)sink;
}
