# Reconcile round — two counters, one measurement

Branch `grind/reconcile`.  Artifact: `artifacts/rebuild/parity-zero.dll`
(map `recon/parity-runs/zero/map/SexLabPPrism.map`), original
`artifacts/SexLabPPrism.dll`, declarations from `recon/declared-mappings.csv`
(92 declared addresses, 84 present in the original inventory).

## Verdict of the investigation: the counters never disagreed on the same population

`tools/real-functions-score.py` restricted to the 991 REAL (unwind-entry)
functions.  `match.py`'s raw line is over all 1,645 inventory rows
(991 real + 654 gap-scan pseudo-functions).  The three denominators reconcile
exactly:

| split | BYTE | INSN | RATIO | MISSING | DECLARED-DIVERGENT | total |
|---|---|---|---|---|---|---|
| all rows (`match.py` raw) | 1219 | 39 | 147 | 203 | 37 | 1645 |
| REAL (991) | 634 | 24 | 125 | 171 | 37 | 991 |
| gap-scan pseudo (654) | 585 | 15 | 22 | 32 | 0 | 654 |

`raw MISSING 203 = real MISSING 171 + gap-scan MISSING 32`.
`real MISSING 171 = 9 declared-but-unbound + 162 undeclared`.

The declared pass's `MISSING: 9` is the declared population, not the real
population: `9` is the number of *declared* originals that could not bind, and
all 9 are inside the 171 real MISSING.  It cannot equal 171 and never could —
162 real MISSING originals carry no declaration at all.  The original claim
"real MISSING must equal the declared MISSING (~9)" compared two different
populations; the real bug was a *consumer* that could not see the fifth verdict.

## What was actually broken (and fixed)

* `DECLARED-DIVERGENT` was already in the JSON `verdict` field, but
  `real-functions-score.py` only knew four verdicts, so its four buckets summed
  to 954 of 991 — the 37 DECLARED-DIVERGENT rows were silently dropped and
  `MISSING 171` appeared with no way to tell it from the declared count.
* An **unbound** declaration (9 rows) had no annotation at all in the JSON: it
  was an anonymous `MISSING` row, so no consumer could recover its symbol or
  the binding reason.

Fixes:
* `tools/match.py` — every declared original now carries `declared`,
  `declared_symbol`, `declared_evidence`, `declared_binding`,
  `declared_map_reason`, `declared_verdict` and `declared_class`
  (`UNBOUND`|`BOUND`|`DECLARED-DIVERGENT`) in the per-function JSON; the report's
  per-tier table has a DECLARED-DIVERGENT column.
* `tools/real-functions-score.py` — DECLARED-DIVERGENT is a first-class bucket;
  the five buckets are asserted to sum to the population; it accepts the JSON
  path as argv[1].
* `tools/reconcile_selftest.py` — pins the cross-tool agreement (12 checks).

## Self-tests

* `tools/map_selftest.py` — 17/17 PASS
* `tools/declared_selftest.py` — 5/5 PASS
* `tools/reconcile_selftest.py` — 12/12 PASS

## The 9 unbound declarations (`declared_class == UNBOUND`)

All 9 share one root cause: the declared body is a **stub / behaviour-only
placeholder** from an earlier round, which the compiler folded to a trivial
function.  The exact map symbol resolves to a rebuild slot that is already
paired (BYTE-MATCH) with another original, and the original's masked bytes are
**absent from the whole rebuild** (byte-hash search over all 2,513 rebuild
functions finds no match; token ratio to the occupied slot ≤ 0.18).

| orig | insn | tier | declared symbol / evidence | occupied slot | disposition |
|---|---|---|---|---|---|
| 0x180022e60 | 112 | plugin | mg5_PublishActiveState (MissingRound2Group2.cpp:31) | 0x180020f20 ← 0x18002dd00 | WRITE (stub) |
| 0x18001e000 | 100 | library | ThreadJoin (MissingRound2Group3.cpp:19) | 0x180026e80 ← 0x18001bfd0 | UNAVAILABLE (std::thread shim) |
| 0x18002e0a0 | 51 | library | LookupAndCallIdFunction2 (:23) | 0x180016c80 ← 0x180018940 | WRITE (stub) |
| 0x180037490 | 21 | plugin | mg2_IdLookupA (MissingGroup2.cpp:284) | 0x180016c80 ← 0x180018940 | WRITE (stub) |
| 0x180037ae0 | 21 | plugin | mg5_RelIdLookup (MissingRound2Group2.cpp:399) | 0x180016c80 ← 0x180018940 | WRITE (stub) |
| 0x1800390b0 | 21 | plugin | IdLookup_615a0 (MissingRound2Group1.cpp:231) | 0x180016c80 ← 0x180018940 | WRITE (stub) |
| 0x180039120 | 21 | plugin | LookupIdGlobal (:120) | 0x180016c80 ← 0x180018940 | WRITE (stub) |
| 0x180055090 | 20 | library | LookupAndCallIdFunction (:276) | 0x180020f20 ← 0x18002dd00 | WRITE (stub) |
| 0x180054d30 | 10 | library | ShutdownUiGlobals (MissingGroup3.cpp:217) | 0x180023a90 ← 0x180038f50 | WRITE (stub) |

Evidence the bodies are stubs, not merely mis-bound:
`mg2_IdLookupA()` calls `mg2_IdDatabaseLookup()` which is `{ return 0; }`, so the
whole body constant-folds to a 2-insn function; `ShutdownUiGlobals()` is
`UiBridge::PushState();  // behaviour-only`; `IdLookup_615a0()` returns 0 when
`Id2Offset({0,0,0}) == 0`.  None of the 9 addresses has a Ghidra decompilation
under `recon/decompiled*`, so none can be hand-written this round; they need a
dedicated decompile-and-write round (the 6 ID-lookup stubs + ShutdownUiGlobals +
mg5_PublishActiveState).

Task-brief addresses vs this population: `0x180047b60`, `0x18003e850` and
`0x18002e000` are DECLARED-DIVERGENT (37-bucket), not among the 9 unbound;
`0x180037fe0` is a declared original at verdict RATIO (library, 45 insn,
→0x180041080).  `0x18002e000` remains the known weak-anchor row (declared
`GrowBSTArray`, ratio 0.1491).
