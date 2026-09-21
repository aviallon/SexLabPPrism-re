## 2026-09-21 — spdlogshape: spdlog instantiation set is IDENTICAL; verdict = packaging, not configuration

Branch: `grind/spdlogshape` @ (see final commit). Worktree `wt/spdlogshape`.
Files: `tools/spdlog_inventory.py`, `docs/spdlog-instantiation-report.md` (new).
No source change — see verdict.

### What I set out to test
The lead: "our SetupLog pattern lacks the elapsed token, so our spdlog build
does not instantiate `spdlog::details::elapsed_formatter` (orig 0x18004a450),
and the fixable cause is the pattern/logger configuration."

### Measurement (ORIG = artifacts/SexLabPPrism.dll, PAR = artifacts/rebuild/parity-r14revert.dll)
- spdlog **type descriptors**: ORIG **96**, PAR **98**. `only-ORIG = 0`,
  `only-PAR = 2` (`msvc_sink` + its `_Ref_count_obj2`).
- `elapsed_formatter`: **4 duration units × 2 padders = 8 TDs in BOTH.**
  The original instantiates elapsed_formatter even though its pattern string
  contains no `%o/%i/%u/%O`.
- Sinks in both: `basic_file_sink<mutex>`, `wincolor_stdout_sink<console_mutex>`,
  `wincolor_sink`, `base_sink`, `sink`. PAR additionally has `msvc_sink`.
- Pattern strings (raw string scan):
  - ORIG: `[%Y-%m-%d %H:%M:%S.%e] [%l] %v` — **exactly** our SetupLog pattern.
  - PAR: the same, **plus** CLNG's `[%T.%e] [%=5t] [%L] %v` (from log::init).
- The 2 library-tier MISSING spdlog rows in `recon/matching.csv` are
  **present in PAR**, not absent: `0x18004a450` is at PAR `0x18005b060`
  (verified by reading the vtable slot out of `rtti-PAR.json` and disassembling);
  `0x180044ca0` has opcode-identical candidates in PAR. This is the round-1
  *pairing* problem, not a missing instantiation.

### Mechanism (why the lead is falsified)
spdlog 1.16 `pattern_formatter-inl.h`: `handle_flag_` is a **member function
template `template<typename Padder>`** whose switch compiles **every** case
body, and `compile_pattern_` calls it for **both** `scoped_padder` and
`null_scoped_padder` unconditionally. So all 35 flag templates × 2 padders plus
the 4 elapsed units × 2 are instantiated at **compile time**, independent of the
runtime pattern. Changing the pattern cannot add or remove an instantiation —
which is why the original has elapsed_formatter with no `%o` in its pattern.

### Verdict
- **Configuration (SetupLog): nothing to fix.** Pattern and sink sets are
  already byte/type-identical to the original.
- **Packaging:** the two real deltas — (a) our *extra* `msvc_sink`/CLNG
  `log::init` that the original's whole-program LTCG eliminated and that we
  cannot reach from SetupLog because `SKSE::Init` is a separate non-`/GL`
  object; (b) our spdlog is a standalone non-`/GL` static lib so its bodies are
  not LTCG-merged and have different prologues/register allocation — are build
  properties.
- Fix, if ever pursued: vendor spdlog into the LTCG unit with
  `header_only=false, wchar=true, std_format=true`. Do **not** reuse the
  `PRISM_LTO_SCOPE=clng|project` knob: round 1 measured it collapsing real
  BYTE 846 → 527. Needs an isolated spdlog-only experiment.

### Request
- **None from `SetupLog`.** I have not touched `src/main.cpp`.
- If the orchestrator wants the 2 pairing artifacts (not matches) cleared, the
  fix belongs to `tools/match.py` / pairing, not to me.
- Ask `ltcg` only if an isolated spdlog-vendoring experiment is authorised.

### Prediction for one build (stated before any build)
Measurement-only re-pair of r14: types appearing = **none new**; spdlog library
MISSING clearing = **2 → RATIO/INSN, 0 BYTE**; shipping config risk = none (no
source change, no build needed).