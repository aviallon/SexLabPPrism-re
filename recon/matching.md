# Matching decompilation scoreboard

Original: `artifacts/SexLabPPrism.dll`  |  Rebuild: `artifacts/rebuild/SexLabPPrism-parity-lto.dll`
Verdict basis: BYTE-MATCH = INSN-MATCH plus identical bytes after masking address-encoding bytes (55708 orig / 67726 rebuild bytes inside function windows, over 1972/2440 instructions); plus base-relocation-table bytes (1533 orig / 1524 rebuild entries) and a trailing-padding strip.

## Overall

- original functions: 2149
- BYTE-MATCH: 1060
- INSN-MATCH: 22
- RATIO: 333
- MISSING: 734
- matching, function-weighted: **49.33%**
- matching, instruction-weighted: **35.78%** (31452/87894 orig instructions)

## Per tier

| tier | funcs | BYTE | INSN | RATIO | MISSING | byte% | insn% |
|---|---|---|---|---|---|---|---|
| plugin | 0 | 0 | 0 | 0 | 0 | 0.0 | 0.0 |
| library | 2149 | 1060 | 22 | 333 | 734 | 49.3 | 35.8 |
| all | 2149 | 1060 | 22 | 333 | 734 | 49.3 | 35.8 |

## Closest to matching (unmatched by bytes)

| orig | name | tier | orig insn | our insn | ratio | first div |
|---|---|---|---|---|---|---|
| 0x1800140c0 |  | library | 29 | 29 | 1.0000 | None |
| 0x180023a80 |  | library | 7 | 7 | 1.0000 | None |
| 0x1800312f0 |  | library | 7 | 7 | 1.0000 | None |
| 0x180031440 |  | library | 5 | 5 | 1.0000 | None |
| 0x180038a90 |  | library | 22 | 22 | 1.0000 | None |
| 0x180048265 |  | library | 13 | 13 | 1.0000 | None |
| 0x18004d0f0 |  | library | 29 | 29 | 1.0000 | None |
| 0x18004f895 |  | library | 85 | 85 | 1.0000 | None |
| 0x18004fa40 |  | library | 47 | 47 | 1.0000 | None |
| 0x1800505c8 |  | library | 19 | 19 | 1.0000 | None |
| 0x180050604 |  | library | 35 | 35 | 1.0000 | None |
| 0x18005074c |  | library | 14 | 14 | 1.0000 | None |
| 0x180050778 |  | library | 17 | 17 | 1.0000 | None |
| 0x180050b64 |  | library | 67 | 67 | 1.0000 | None |
| 0x180050c7c |  | library | 34 | 34 | 1.0000 | None |

