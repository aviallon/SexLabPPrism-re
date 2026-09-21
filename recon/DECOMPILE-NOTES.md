# decompile-agent notes — round 2 (missing-metric first)

Goal of this round: make every MISSING function with a recovered name a real body
first, then byte-match. These notes cover the two things that make that possible:
a full-coverage Ghidra export for the 22 named MISSING functions, and an
evidence-based audit that bounds the 732 unnamed MISSING functions.

## Deliverables

| path | what |
|---|---|
| `build/recon/decompiled-new/0x<addr>_<name>.c` | Ghidra C for all 22 addresses in `build/recon/implement-list.csv` |
| `build/recon/decompiled-new/MANIFEST.csv` | `address,name,insn,file,status,total_lines` |
| `build/recon/missing-classified.csv` | audit of the 732 unnamed MISSING functions |
| `tools/ghidra/PrismExport.java` | Ghidra headless post-script that produced the export |
| `tools/audit-missing.py` | one-shot objdump + PE/RTTI reference audit |

## 1. Re-running the decompilation pipeline

Ghidra 12.1.2, headless, Nix only. The whole 656 KB DLL analyses in ~2 minutes.

```bash
cd ~/Projects/SexLabPPrism-re
# one-shot full analysis + export of the 22 implement-list addresses
nix shell nixpkgs#ghidra -c ghidra-analyzeHeadless \
  "$PWD/build/recon/ghidra-proj" Prism \
  -import "$PWD/artifacts/SexLabPPrism.dll" \
  -scriptPath "$PWD/tools/ghidra" \
  -postScript PrismExport.java \
    "$PWD/build/recon/decompiled-new" \
    "$PWD/build/recon/implement-list.csv" \
    "$PWD/recon/functions.txt"
# later re-exports reuse the saved project (no re-analysis):
nix shell nixpkgs#ghidra -c ghidra-analyzeHeadless \
  "$PWD/build/recon/ghidra-proj" Prism -process SexLabPPrism.dll \
  -scriptPath "$PWD/tools/ghidra" \
  -postScript PrismExport.java \
    "$PWD/build/recon/decompiled-new" \
    "$PWD/build/recon/implement-list.csv" \
    "$PWD/recon/functions.txt"
```

`PrismExport.java` renames functions from `recon/functions.txt`, decompiles every
target, and writes the .c plus a MANIFEST row. `status` is `clean` unless Ghidra
reports a real failure (`Could not recover jumptable`, `Too many branches`,
`Treating indirect jump as call`, `Unable to decompile`, `Bad instruction`,
`LowLevelError`, `(bad)`, pcode warnings). Ghidra's benign `Function: ...
replaced with injection` and `Globals starting with ... overlap` notes are
explicitly ignored — they do not degrade the C.

## 2. Export coverage and honesty

All 22 addresses exported. **21 clean, 1 degraded:**

* `0x18002cc20` `.??AV?$_Func_impl_no_alloc@...H@std@@::vfunc[2]` (5 insns) —
  jump table not recovered: `Could not recover jumptable ... Too many branches`,
  `Treating indirect jump as call`. It is a 2-3 line MSVC `std::function`
  dispatcher; the C is usable but the indirect branch is a placeholder.

No address was missing and no decompile timed out. Addresses that look
switch-heavy or STL-heavy (the `_Func_impl_no_alloc` family, the
`_Fmt_iterator_buffer` vtable at `0x180044740`) decompile to thin wrappers, so
treat their C as a signature sketch, not as a body to transcribe.

## 3. The audit method (`tools/audit-missing.py`)

The whole image is disassembled **once** (`objdump -d -M intel
--no-show-raw-insn artifacts/SexLabPPrism.dll`, 98,920 instructions) and each of
the 732 functions is sliced by `orig_insn` from `missing-unnamed.csv`. For every
instruction the referenced addresses (`call`/`jmp` targets and rip-relative
`# 0x...` data addresses) are collected. A function is:

* **plugin** — it references plugin-owned data: a literal from the plugin string
  set (`prisma/slpp/sexlab/catalog/freecam/focusrecovery/papyrus/...`), a plugin
  RTTI type descriptor/vtable/COL (`InputSink`, `MenuVisibilitySink`,
  `?A0xbb2e73b6`), or calls a plugin function already identified from
  `functions.txt`/`implement-list`/`recon/decompiled`/RTTI vtables.
* **library** — it references an imported symbol (IAT VA, parsed from the PE
  import directory) or a library literal (`spdlog`, `fmt::`, `std::`, `CLNG`,
  `Basic_string`, ...).
* **uncertain** — neither. The earlier round's `tier` column is reported in the
  `evidence` field (`prior=plugin` / `prior=library`) but **never drives the
  classification**, because round 1 already showed the pairing/tier prior is
  unreliable. Classification is reference-only.

## 4. What the audit found

| classification | funcs | insns |
|---|---|---|
| plugin | **9** | **209** |
| library | 52 | 2,553 |
| uncertain | 671 | 8,786 |
| **total** | **732** | **11,548** |

Largest plugin-classified findings (real bodies, not thunks):

| addr | insns | evidence |
|---|---|---|
| `0x18002d170` | 94 | plugin string `window.slppCatalogReset({});window.slppCatalogProgress(0,{});` + calls `anonymous-namespace` plugin func |
| `0x18002d340` | 67 | plugin string `window.slppCatalogDone({});` + calls plugin func |
| `0x18002e340` | 22 | plugin string `SexLabPPrism` |
| `0x180016d20` | 17 | calls `FocusRecovery::anonymous-namespace::CloseOwnedConsole` |
| `0x180016c00` | 2 | thunk to `FocusRecovery::...::Finalize` |
| `0x18002cfc0` | 2 | thunk to `RequestSearchInput` |
| `0x18002d4a0` | 2 | thunk to `RequestSearchInput` |
| `0x18002d4d0` | 1 | thunk to `ApplyPresentation` |
| `0x18002d4e0` | 2 | thunk to `RequestSearchInput` |

These 9 are appended to `missing-classified.csv` (plugin rows first) so they can
be folded into the implement list.

## 5. Honest limits

* **The prior and the evidence disagree.** Of the 732, the earlier `tier` says
  68 plugin (3,251 insns); reference evidence confirms only 9. 41 of the 68
  (`prior=plugin`, 1,203 insns; largest `0x18004d830` 102, `0x180013f20` 99,
  `0x180023230` 77) reference **no** plugin string, RTTI or known plugin
  function at all. They are reported `uncertain`, not plugin. Most of them look
  like string-free STL/CRT runtime helpers (`_invoke_watson`, security cookie,
  condition-variable wait) and the prior is probably wrong for them; a few could
  be genuine string-free plugin helpers that cannot be decided statically.
* **No distinctive reference ⇒ not classifiable.** 671 functions (8,786 insns,
  76% of the audited instruction volume) are small stubs, pure control flow,
  arithmetic, or CRT helpers whose only references are addresses internal to the
  same function. Static references cannot separate plugin helper from STL
  helper for these; pretending otherwise would be a guess.
* **ICF-folded twins share code.** The linker folded identical COMDATs, so one
  address can serve both a plugin and a library call site (the round-1 `icf`
  anchor column in `recon/matching.csv`). A classification therefore describes
  the *reference site*, and a folded body may legitimately be claimed by both
  tiers. This is why `_invoke_watson`-based functions read `library` even when
  the prior says `plugin`.
* **Ghidra's `.pdata` gap-scan noise is excluded.** These 732 are the real
  `.pdata` MISSING set; the gap-scan pseudo-functions are not in this audit.

## 6. Reproducing the audit

```bash
cd ~/Projects/SexLabPPrism-re
nix shell nixpkgs#binutils -c objdump -d -M intel --no-show-raw-insn \
  artifacts/SexLabPPrism.dll > build/recon/objdump-text.txt
python3 tools/audit-missing.py     # writes build/recon/missing-classified.csv
```