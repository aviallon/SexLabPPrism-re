# pairfix verification — no false pairs introduced (2026-09-21)

Branch: `grind/pairfix`.  Tool: `tools/pairfix_infolding.py` (also reachable as
`tools/match.py --identity --infolding`, which only annotates; it never adds,
removes or mutates a pair).

## 1. Pairing/verdict output is byte-identical with and without --infolding

```
pairs/verdict tuples: infolding 1565 plain 1565 symmetric-diff 0
infolding fields present: 88
0x1800250f0 -> ('MISSING', 'PRESENT-INFOLDED', None)
```

`--infolding` adds a `missing_class` field to the 88 MISSING rows and nothing
else.  `verdict` for an infolded function stays `MISSING`, so it is never
counted as BYTE/INSN/RATIO; the class is a separate bucket.

Commands (artifact = newest in `artifacts/rebuild/`,
`parity-r14revert.dll`, mtime 2026-09-21 16:24):

```
python3 tools/match.py --identity --infolding --orig .../SexLabPPrism.dll --new .../parity-r14revert.dll
python3 tools/match.py --identity              --orig .../SexLabPPrism.dll --new .../parity-r14revert.dll
```

## 2. Harness self-test (`tools/match.py --self-test`)

```
self-test: 5/5 passed
  PASS identical binary: BYTE-MATCH 1565/1565 (expected 100%)
  PASS mutation: only 0x180001010 differs, all others BYTE-MATCH
  PASS deletion: 0x180001010 absent from inventory (MISSING)
  PASS relocation masking (real pair): 0x180001000 BYTE-MATCH though raw bytes differ
  PASS relocation tables parsed: 1533 orig entries, 1514 rebuild entries
```

## 3. Classifier self-test (`tools/pairfix_infolding.py --self-test`)

```
PASS unique literal in paired function -> INFOLDED
PASS literal shared by two originals is NOT infolded evidence
PASS literal in unpaired rebuild fn -> PRESENT-UNPAIRED
PASS a non-MISSING original is never classified
PASS no anchor -> UNKNOWN
self-test: 5/5 passed
```

The negative cases are the point: an anchor is rejected unless it is
*distinctive* (a literal referenced by exactly one original, or an
RTTI/callee referenced by at most two), and a function that already has a
paired verdict is never re-classified.

## 4. The premise, checked end-to-end

`0x1800250f0` (302 insns, the JS body emitting `window.slppVitals` +
`window.slppState`) is PRESENT-INFOLDED: its distinctive literal
`window.slppState(` (fuzzy-bound to our `window.slppState({});`) is referenced
by rebuild `0x180028b90`, which is paired to original `0x180024450` (the
original `PushState`).  Source `src/UiBridge.cpp:56` states this explicitly:
"0x1800250f0 (302 insns): the outlined body of `PushState(void)::<lambda_1>`".
The rebuild emits the same two JS commands (`window.slppState({});`,
`window.slppVitals({});`) from one function, so no separate function with the
anchor exists to pair.
