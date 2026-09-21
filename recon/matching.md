# Matching decompilation scoreboard

Original: `artifacts/SexLabPPrism.dll`  |  Rebuild: `artifacts/rebuild/SexLabPPrism-parity-lto.dll`
Verdict basis: BYTE-MATCH = INSN-MATCH plus identical bytes after masking address-encoding bytes (55708 orig / 67574 rebuild bytes inside function windows, over 1972/2454 instructions); plus base-relocation-table bytes (1533 orig / 1524 rebuild entries) and a trailing-padding strip.

## Overall

- original functions: 2149
- BYTE-MATCH: 1070
- INSN-MATCH: 16
- RATIO: 329
- MISSING: 734
- matching, function-weighted: **49.79%**
- matching, instruction-weighted: **36.11%** (31737/87894 orig instructions)

## Per tier

| tier | funcs | BYTE | INSN | RATIO | MISSING | byte% | insn% |
|---|---|---|---|---|---|---|---|
| plugin | 399 | 180 | 1 | 144 | 74 | 45.1 | 26.5 |
| library | 1750 | 890 | 15 | 185 | 660 | 50.9 | 41.3 |
| all | 2149 | 1070 | 16 | 329 | 734 | 49.8 | 36.1 |

## Closest to matching (unmatched by bytes)

| orig | name | tier | orig insn | our insn | ratio | first div |
|---|---|---|---|---|---|---|
| 0x180023a80 |  | library | 7 | 7 | 1.0000 | None |
| 0x1800312f0 |  | library | 7 | 7 | 1.0000 | None |
| 0x180031440 |  | library | 5 | 5 | 1.0000 | None |
| 0x180048265 |  | library | 13 | 13 | 1.0000 | None |
| 0x1800505c8 |  | library | 19 | 19 | 1.0000 | None |
| 0x180050604 |  | library | 35 | 35 | 1.0000 | None |
| 0x18005074c |  | library | 14 | 14 | 1.0000 | None |
| 0x180050778 |  | plugin | 17 | 17 | 1.0000 | None |
| 0x180050b64 |  | library | 67 | 67 | 1.0000 | None |
| 0x180050c7c |  | library | 34 | 34 | 1.0000 | None |
| 0x180050e68 |  | library | 155 | 155 | 1.0000 | None |
| 0x180051498 |  | library | 39 | 39 | 1.0000 | None |
| 0x180051690 |  | library | 9 | 9 | 1.0000 | None |
| 0x180054ef0 |  | library | 28 | 28 | 1.0000 | None |
| 0x180054fb0 |  | library | 13 | 13 | 1.0000 | None |

