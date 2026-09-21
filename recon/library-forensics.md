# Library-tier MISSING forensics (round 6, grind/libforensics)

Inputs: `recon/matching/per-function.json` (identity pairing, parity-r5), the
original `artifacts/SexLabPPrism.dll`, rebuild `artifacts/rebuild/parity-r5.dll`.
Tools: `tools/libforensics.py`, `tools/vtable-analog.py` (both import
`tools/match.py`/`tools/parity.py`; no PE parsing is re-implemented).

## 0. The headline correction: 426 of the "660 library MISSING" are gap-scan noise

`recon/matching/per-function.json` tiers every function, including the 654
gap-scan pseudo-functions (bytes in .text with no `.pdata` unwind record).
Splitting the 660 library MISSING by `source` from
`recon/parity-parity/per-function.json`:

| source | functions | instructions |
|---|---|---|
| `pdata` (real) | **234** | **7,682** |
| `gap` (no unwind) | 426 | 1,028 |

The defensible library-MISSING population is therefore **234 real functions /
7,682 instructions**, not 660. All H1–H4 arithmetic below uses the 234.

## 1. Sample: 30 largest real library MISSING (by original instruction count)

`best` = highest token-stream `SequenceMatcher.ratio` against *any* rebuild
function (mnemonic prefilter ≥0.5, length ±40%); this is deliberately not the
pairing result. Full table: `recon/library-forensics-sample.csv`,
`recon/library-forensics-bestmatch.json`, `recon/library-forensics-real-split.json`.

| # | orig addr | insn | code refs | data ref | best analog | ratio | hypothesis |
|---|---|---|---|---|---|---|---|
| 1 | 0x18000d194 | 356 | 0 | yes | 0x180014c14 | 0.941 | H3 (see §3: link-time disp32 only) |
| 2 | 0x180013a20 | 349 | 1 | yes | — | 0.145 | H4 |
| 3 | 0x18003fec0 | 306 | 1 | yes | — | 0.465 | H4 (std::format machinery) |
| 4 | 0x18003d710 | 303 | 1 | yes | 0x18004d2c0 | 0.718 | H3 |
| 5 | 0x1800250f0 | 302 | 1 | yes | — | 0.227 | H4, but plugin (`window.slppVitals`) |
| 6 | 0x1800466d0 | 183 | 2 | yes | 0x180056a60 | 0.501 | H3 |
| 7 | 0x180046910 | 183 | 2 | yes | 0x180056a60 | 0.501 | H3 (ICF pair of #6) |
| 8 | 0x180044740 | 174 | 1 | yes | — | 0.301 | H4 (std::format machinery) |
| 9 | 0x18003af80 | 173 | 1 | yes | 0x18004abc0 | 0.701 | H3 |
| 10 | 0x1800464d0 | 166 | 4 | yes | 0x180056860 | 0.772 | H3 (prologue only) |
| 11 | 0x180024450 | 152 | 1 | yes | — | 0.370 | H4, but plugin (`window.slppSetSearchQuery`) |
| 12 | 0x18004ce40 | 146 | 1 | yes | — | 0.428 | H4 |
| 13 | 0x18003aa1b | 136 | 0 | yes | — | 0.156 | H4 |
| 14 | 0x18003eb30 | 134 | 1 | yes | 0x180019480 | 0.575 | H3 |
| 15 | 0x180005cee | 116 | 0 | yes | 0x18001065e | 0.957 | H3 (link-time disp32 only) |
| 16 | 0x1800058ad | 113 | 0 | yes | 0x18001986d | 0.956 | H3 (link-time disp32 only) |
| 17 | 0x180001d94 | 112 | 0 | yes | — | 0.352 | H4 |
| 18 | 0x1800471ca | 111 | 0 | yes | — | 0.349 | H4 |
| 19 | 0x18001e000 | 100 | 2 | yes | — | 0.182 | H4 |
| 20 | 0x18003cd9e | 93 | 0 | yes | — | 0.320 | H4 |
| 21 | 0x180014dfb | 84 | 0 | yes | — | 0.147 | H4 |
| 22 | 0x180054970 | 83 | 0 | yes | — | 0.522 | H3 |
| 23 | 0x18003ca10 | 73 | 0 | yes | — | 0.453 | H4 |
| 24 | 0x180047e5e | 73 | 0 | yes | — | 0.099 | H4 |
| 25 | 0x18002d340 | 67 | 0 | yes | 0x18005f410 | 0.365 | H4 (NativeFunction vtable bodies) |
| 26 | 0x180016c10 | 66 | 0 | yes | — | 0.279 | H4 |
| 27 | 0x18003a71d | 64 | 0 | yes | — | 0.404 | H4 |
| 28 | 0x180046dea | 61 | 0 | yes | — | 0.167 | H4 |
| 29 | 0x180047008 | 60 | 0 | yes | — | 0.454 | H4 |
| 30 | 0x1800190b0 | 57 | 0 | yes | — | 0.387 | H4 |

No sampled function is H2 (dead-code eliminated): all are referenced by code
and/or an .rdata vtable/type descriptor. `tools/vtable-analog.py` finds only
33 of the 660 sit in a recoverable vtable slot, so virtual dispatch is *not*
the dominant mechanism.

## 2. Mechanical H1–H4 split over all 234 real library MISSING

Rules (mechanical, in `tools/libforensics.py`):
* **H1 folded/ICF** — the normalised token stream appears contiguously inside a
  rebuild function, or the rebuild contains a masked-byte-identical twin
  (`recon/library-forensics-icf.json`), or best analog ≥ 0.98.
* **H3 shape** — best analog ratio in [0.5, 0.98): the counterpart exists but
  the code shape differs.
* **H2 eliminated** — best analog < 0.5 and *no* code reference and *no* data
  reference in the original.
* **H4 absent** — best analog < 0.5 but referenced (vtable/data/code): our
  build never compiled that path with that shape.

| hypothesis | functions | instructions |
|---|---|---|
| H1 folded/ICF | 9 | 50 |
| H2 eliminated | 0 | 0 |
| H3 shape / instantiation | **122** | **3,657** |
| H4 genuinely absent | **103** | **3,975** |
| **total** | **234** | **7,682** |

Extrapolation note: because the 426 gap-scan functions are excluded, none of
this arithmetic should be scaled back up to "660" — 65% of that figure was
measurement noise.

## 3. The largest fixable finding: link-time displacement constants are not canonicalised

`0x18000d194` (356 insns, largest real library MISSING) has a rebuild analogue
at `0x180014c14` (356 insns) at ratio **0.941**, and their token diff is
*exclusively* memory-operand displacement constants:

```
orig: movzx eax,BYTE PTR [rbp+rsi*1+0x5bab0]   new: ...[rbp+rsi*1+0x98fb0]
orig: cmp rax,QWORD PTR [rsi+rbp*8+0x5bbf0]    new: ...[rsi+rbp*8+0x990f0]
```

`tools/parity.py::_norm_operand` canonicalises only `[rip+disp]` and `[0x...]`
operands. A `[base+index*scale+disp32]` operand falls through to `HEX_RE.sub`,
which canonicalises only when `image_base <= v < image_end` — a raw RVA such as
`0x5bab0` is *below* image_base, so it is left literal. `match.py::ins_mask_offsets`
has the same gap, so these bytes are not masked either. The displacement is the
link-time distance between a RIP-loaded symbol and a static table; it changes
whenever `.rdata` layout changes, exactly like a relocation.

Same mechanism at `0x180005cee`→`0x18001065e` (0.957, 116 insns) and
`0x1800058ad`→`0x18001986d` (0.956, 113 insns).

Scope: 25 real library functions (6 MISSING / 991 insns, 19 RATIO / 6,542
insns) contain a ≥5-hex-digit memory displacement. A conservative fix
(canonicalise/mask bracketed `0x…` displacements above a threshold) should turn
several of the 19 RATIO functions into INSN/BYTE and pair the three near-twins.

## 4. What cannot be fixed from the build configuration

* **H4 (103 functions / 3,975 insns)** — no token analogue at any ratio. Two
  identifiable sub-classes:
  * std::format internals (`0x18003fec0`, `0x180044740`, and 136 more referencing
    the `@…` compile-time format-signature literals, e.g. "integral cannot be
    stored in char"). These depend on the exact `std::format` argument types at
    each call site — a source-level detail, not an xmake flag.
  * plugin bodies mis-tiered as library because they do not reference a
    `SexLabPrism`/`Prisma` string directly (`0x1800250f0` `window.slppVitals`,
    `0x180024450` `window.slppSetSearchQuery`). Tier is a heuristic; these are
    implementation gaps owned by `implplugin`/`matchratio`.
* The remaining H4 functions have no recovered name, no string reference and no
  vtable slot, so they cannot be attributed from the binary alone. Fixing them
  would require the original build environment or PDB, or naming them one by
  one from decompilation.
* **H2 = 0 for real functions.** The earlier "303 eliminated" figure was
  entirely gap-scan 2-instruction thunks, not real dead code.
