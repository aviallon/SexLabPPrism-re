# Matching decompilation scoreboard

Original: `artifacts/SexLabPPrism.dll`  |  Rebuild: `artifacts/rebuild/SexLabPPrism-parity-lto.dll`
Verdict basis: BYTE-MATCH = INSN-MATCH plus identical bytes after masking address-encoding bytes (71874 orig / 92968 rebuild bytes inside function windows, over 1580/2278 instructions); plus base-relocation-table bytes (1533 orig / 1673 rebuild entries) and a trailing-padding strip.

## Overall

- original functions: 1645
- BYTE-MATCH: 861
- INSN-MATCH: 40
- RATIO: 271
- MISSING: 445
- DECLARED-DIVERGENT: 28
- matching, function-weighted: **52.34%**
- matching, instruction-weighted: **49.00%** (43076/87918 orig instructions)

## Per tier

| tier | funcs | BYTE | INSN | RATIO | MISSING | byte% | insn% |
|---|---|---|---|---|---|---|---|
| plugin | 498 | 310 | 5 | 142 | 34 | 62.2 | 53.2 |
| library | 1147 | 551 | 35 | 129 | 411 | 48.0 | 43.2 |
| all | 1645 | 861 | 40 | 271 | 445 | 52.3 | 49.0 |

## Closest to matching (unmatched by bytes)

| orig | name | tier | orig insn | our insn | ratio | first div |
|---|---|---|---|---|---|---|
| 0x180010bf0 |  | library | 16 | 16 | 1.0000 | None |
| 0x180031520 |  | library | 19 | 19 | 1.0000 | None |
| 0x180037e90 |  | plugin | 42 | 42 | 1.0000 | None |
| 0x180037f90 |  | library | 3 | 3 | 1.0000 | None |
| 0x180038270 |  | plugin | 5 | 5 | 1.0000 | None |
| 0x180038290 |  | plugin | 5 | 5 | 1.0000 | None |
| 0x1800382c0 |  | plugin | 5 | 5 | 1.0000 | None |
| 0x180047e20 |  | plugin | 5 | 5 | 1.0000 | None |
| 0x180051fb0 |  | library | 13 | 13 | 1.0000 | None |
| 0x180052120 |  | library | 13 | 13 | 1.0000 | None |
| 0x180052470 |  | library | 13 | 13 | 1.0000 | None |
| 0x180052550 |  | library | 13 | 13 | 1.0000 | None |
| 0x1800526b0 |  | library | 13 | 13 | 1.0000 | None |
| 0x180052790 |  | library | 13 | 13 | 1.0000 | None |
| 0x180052840 |  | library | 60 | 60 | 1.0000 | None |

