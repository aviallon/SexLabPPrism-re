# Missing-function triage (grind/triage, round 3→4)

Inputs: `build/recon/matching/per-function.json` (identity-paired verdicts),
`build/recon/missing-classified.csv` (round-3 reference audit),
`artifacts/SexLabPPrism.dll`, `artifacts/rebuild/parity-r4.dll`.
Tool: `tools/triage_missing.py` (imports `tools/match.py` / `tools/parity.py`
for the function inventory and normalisation; one objdump run per binary).

## Scope

`per-function.json` has **2,149** structural functions; **1,495** are "real"
(have a `.pdata` RUNTIME_FUNCTION / unwind entry). The identity matcher reports
**734 MISSING** overall, of which **291 are real** (10,866 instructions). That
is the set triaged here.

## Class counts (build/recon/missing-triage.csv)

| class | funcs | insn | meaning |
|---|---:|---:|---|
| PRESENT-UNPAIRED | 5 | 318 | rebuild already contains an excellent match (ratio ≥ 0.95) elsewhere; pairing bug, do not implement |
| ABSENT-LIBRARY | 19 | 1,625 | original references library internals (CRT `_invoke_watson`/`terminate`/`abort`, `MSVCP140!ctype`); same library sources, so a build/version/shape difference |
| ABSENT-PLUGIN | 50 | 2,313 | tier=plugin by exports/RTTI reachability, or round-3 audit referenced plugin data, and no rebuild function matches ≥ 0.95; implement |
| UNCERTAIN | 217 | 6,610 | no distinctive reference and/or no confident match; cannot honestly attribute |

**Really missing (ABSENT-PLUGIN) = 50 / 291 = 17%.** The other 83% is either
already in the binary but unpaired (2%), a library build/shape difference (7%),
or unadjudicable from references alone (75%).

By the fixed tier classifier the 291 split **plugin 56 (3,322 insn) / library 230
(7,226 insn) / 5 PRESENT-UNPAIRED**. So the MISSING metric is overwhelmingly a
*library*-tier problem, not a plugin one — only ~19% of the missing mass is
plugin-owned.

### PRESENT-UNPAIRED (5)

| orig | insn | best rebuild match |
|---|---:|---|
| 0x1800058ad | 113 | 0x18001986d ratio 0.956 |
| 0x180005cee | 116 | 0x18001065e ratio 0.957 |
| 0x180010070 `.?AVexception@std@@::vtable[0]` | 21 | 0x180003e90 ratio 0.952 |
| 0x1800102f0 `.?AVsystem_error@std@@::vtable[0]` | 23 | 0x180003e30 ratio 0.957 |
| 0x180017e30 | 45 | 0x180007950 ratio 0.978 |

Each has exactly **one** rebuild function ≥ 0.95, so the high ratio is a specific
twin, not a generic-template coincidence. Action: add/repair the identity anchor
so the matcher pairs them; these should drop straight out of MISSING.

### Top ABSENT-PLUGIN targets (all 50 in the CSV)

| addr | insn | name |
|---|---:|---|
| 0x18001f0b0 | 167 | `anonymous-namespace::InputSink::ProcessEvent` |
| 0x180026790 | 167 | `anonymous-namespace::ApplyVanillaHUDVisibility` |
| 0x18001e740 | 123 | — (tier=plugin reachable; best ratio 0.585) |
| 0x180023e90 | 114 | `anonymous-namespace::RequestSearchInput` |
| 0x180013760 | 110 | `FocusRecovery::anonymous-namespace::Finalize` |
| 0x18004d830 | 102 | — |
| 0x180013f20 | 99 | — |
| 0x1800146e0 | 94 | `FocusRecovery::anonymous-namespace::VerifyCleanup` |
| 0x18002d170 | 94 | JsCatalogReset lambda (plugin string `window.slppCatalogReset…`) |
| 0x180023230 | 77 | — |
| 0x180006110 | 73 | — |
| 0x180014428 | 68 | `FocusRecovery::anonymous-namespace::StartConsolePulse` |
| 0x18002d340 | 67 | JsCatalogDone lambda (plugin string `window.slppCatalogDone…`) |
| 0x18004dfa0 | 67 | — |
| 0x18003aca0 | 65 | — |
| 0x1800145c5 | 57 | `FocusRecovery::anonymous-namespace::VerifyCleanup` (second chunk) |
| 0x1800196b0 | 55 | — |
| 0x180013920 | 53 | — |
| 0x180012a60 | 49 | — |
| 0x180047500 | 47 | — |

Caution for implgap: several of the named targets (InputSink::ProcessEvent,
ApplyVanillaHUDVisibility, RequestSearchInput, FocusRecovery Finalize/
VerifyCleanup/StartConsolePulse) are exactly what round-3 `implui` claims to have
implemented. Against **parity-r4.dll** they have no ≥ 0.95 token match, and the
top two share < 0.30 token multiset with *every* rebuild function — i.e. they are
not in this dll. Before writing new bodies, check whether the round-3 source
already contains them; if it does, the fix is to get them into the next build,
not to rewrite them.

## Deliverable 2 — tier classifier fix (tools/match.py)

**Symptom.** `match.py --identity` printed `plugin 0 / library 2149`; every
`per-function.json` row had `"tier": "library"`, hiding the plugin side.

**Root cause.** `load_orig_meta()` read the *external* report
`recon/parity-parity/per-function.json` (written by a full `tools/parity.py`
run) through a **cwd-relative** path. When `match.py` ran from `build/`, the
relative path did not exist, `tier_map` came back empty, and every lookup fell
through to the hard-coded default `"library"`. (`pair_identity.pair_indices`
also reads a `tmap` from that file — that read is dead code, the variable is
never used, so it never affected pairing.)

**Fix.** `match.py` now computes the tiers itself with `compute_tiers()` after
building the original inventory, using exactly `parity.classify()`'s definition:
original **exports** + **plugin string/RTTI reference addresses**
(`parity.plugin_ref_addresses`) + the **registration-table** refs + **BFS over
direct calls** from those roots, with library-named functions as a hard stop.
The name input is `recon/functions.json` + recovered `__cdecl` signatures, i.e.
the same inputs the reference run used — deliberately **not** `recon/symbols.csv`
(feeding the symbol map makes RTTI/slot names hit the library-name stop list and
shrinks the plugin tier to 272/1877).

**Verification (before → after).**

| | plugin | library |
|---|---:|---:|
| before (tool output, cwd=build) | 0 | 2149 |
| after | **399** | **1750** |
| historical structural reference (`recon/parity-parity/per-function.json`) | 399 | 1750 |

Agreement with the reference is **2149/2149 functions exact**. End-to-end
`match.py --identity` on `parity-r4.dll` (sandbox `/tmp/triage-verify/run2.log`)
now prints:

```
  plugin    399 funcs  BYTE= 175 INSN=   5 RATIO= 147 MISSING=  72
  library  1750 funcs  BYTE= 885 INSN=  17 RATIO= 186 MISSING= 662
```

so the 0-plugin delusion is gone and the plugin side of the scoreboard is
visible again. (The 734 MISSING here is over all 2149 structural functions; the
291 triaged above are the `.pdata`-real subset.)

Note (from implgap): the original has no `fmt` symbols, so this build uses
`std::format` (`SPDLOG_USE_STD_FORMAT`, `xmake.lua:32`); library-shape work
should assume std-format, not fmt.

## Honest limits

1. **ICF folding.** 896 of the identity pairs carry an `icf` anchor: several
   original functions can legitimately share one rebuild body, so "present
   elsewhere" identifies a body but not which original it belongs to. Presence
   is real; ownership is not resolvable from tokens alone.
2. **Generic templates / false "present".** ratio ≥ 0.95 across all functions can
   hit the same template body repeated in many places. Guard: `triage_missing.py`
   counts how many rebuild functions are ≥ 0.95 for each original and flags
   `[generic-template risk]` when > 4; all 5 PRESENT-UNPAIRED hits have exactly
   one such twin, and exact token-tuple matches are found by dictionary lookup
   rather than fuzzy search.
3. **No distinctive reference ⇒ no class.** 217 of 291 (6,610 insn) have neither
   a plugin string/RTTI ref nor a library-internal ref; tier cannot be decided
   from references, and they stay UNCERTAIN rather than guessed.
4. **ratio < 0.95 is not proof of absence.** The original was LTCG-built; the
   same source can inline differently and produce a very different token stream
   (one ABSENT-PLUGIN candidate merely reaches 0.585), so "no ≥ 0.95 match"
   means "not catalogued on today's rebuild", not "definitely gone".
5. **Reference extraction is incomplete.** `build/recon/objdump-text.txt` has RIP
   comments on only ~3% of lines, so library-internal references that appear
   only as RIP operands are undercounted; the ABSENT-LIBRARY count (19) is a
   lower bound.