# undeclared-C — last-third of the 162 no-declaration rows (grind/undeclared-c)

## 2026-09-23 — slice = 54 addresses 0x1800484d0..0x180053f60; ALL library-provided

- Branch `grind/undeclared-c` (fresh from `main` @ a055c33). Files added:
  `src/missing/MissingRoundNotDeclaredC.cpp` (evidence, NO bodies),
  `recon/undeclared-c-evidence.csv` (machine-readable evidence), this note.
- Reproduced the worklist from `build/recon/matching/per-function.json`
  (orig `artifacts/SexLabPPrism.dll`, new `artifacts/rebuild/parity-zero.dll`):
  real (.pdata) MISSING = **171**, minus `build/recon/declared-mappings.csv`
  = **162** undeclared, 54 per third. My ascending-address last third is
  **0x1800484d0..0x180053f60, 54 funcs / 5145 insns** — 53 library-tier + 1
  plugin-tier (0x18004e1c0, actually spdlog too).
- **Decompiled all 54** clean with Ghidra 12.1.2 (cached project copy,
  `tools/ghidra/PrismExport.java`): `build/recon/decompiled-uc/`,
  clean=54 degraded=0 incomplete=0 notfound=0.
- **Verdict: 54/54 library-provided, 0 plugin bodies to write.**
  - 45/54 proven by RTTI vtable walk of the ORIGINAL: slot-1 `format()` of
    `spdlog::details::<X>_formatter<...>`, provider
    spdlog v1.16.0 `include/spdlog/pattern_formatter-inl.h` (lines in the CSV).
  - 0x18004d240 `spdlog::details::fmt_helper::pad2` (fmt_helper.h:84);
    0x18004de30 `spdlog::details::os::utc_minutes_offset` (os-inl.h:247);
    0x18004e1c0 `spdlog::details::file_helper::open` (file_helper-inl.h:26).
  - 6 x 13-insn (0x180052e80, 0x180053010, 0x1800532d0, 0x1800534e0,
    0x180053b00, 0x180053f60): MSVC STL `<format>` cleanup funclets
    (flag-clear + std::string dtor, zero direct call/jmp refs in the image).
- **These are NOT absent from our rebuild.** The same RTTI vtables exist in
  `parity-zero.dll`; 45/54 have a rebuild counterpart, e.g.
  orig `full_formatter::format` 0x18004c400 -> rebuild **0x1800649e0**;
  orig `A_formatter<scoped_padder>::format` 0x1800484d0 -> rebuild **0x18005fe90**.
  So this slice is a *pairing/binding* problem, not a missing-code problem:
  bind by the dependency's own mangled symbol (or by these addresses), NOT by a
  new body.
- **Declarations: none added, deliberately.** No block is bindable to a plugin
  body, and `tools/extract_declared.py --out /tmp/...` is unchanged at 92 rows
  (74 RECONSTRUCTED + 18 PRESENT-UNPAIRED) after my file, i.e. no false binding.
- **Force-link: none** (no bodies -> nothing for /OPT:REF to strip).
- **What I need from main:** for this slice, have the pairing step bind the 45
  orig->rebuild address pairs in `recon/undeclared-c-evidence.csv`. Do not run a
  CI build for my branch; there is no code change to test.
