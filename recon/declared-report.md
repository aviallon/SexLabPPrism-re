# Declared pairs — audit (DECLARED-DIVERGENT is NOT a match)

declarations in CSV: 72
- BYTE-MATCH: 6
- DECLARED-DIVERGENT: 28
- MISSING: 25

declared rows that are REAL (unwind-entry) functions: 72
- BYTE-MATCH: 6 (179 insn)
- DECLARED-DIVERGENT: 28 (1307 insn)
- MISSING: 25 (2155 insn)

## DECLARED-DIVERGENT rows (claim of intent, no match)

| orig | symbol | src evidence | new | orig insn | our insn | ratio |
|---|---|---|---|---|---|---|
| 0x18003eb30 | WriteAlignedField | src/missing/MissingRound2Group3.cpp:147 | 0x180055750 | 134 | 134 | 0.6119 |
| 0x18003e400 | mg5_WriteAlignedFormat | src/missing/MissingRound2Group2.cpp:259 | 0x1800095b0 | 115 | 116 | 0.6840 |
| 0x18003e6e0 | FormatAlignedField | src/missing/MissingRound2Group1.cpp:251 | 0x180009720 | 115 | 116 | 0.6840 |
| 0x180022e60 | mg5_PublishActiveState | src/missing/MissingRound2Group2.cpp:311 | 0x18000dca0 | 112 | 140 | 0.7937 |
| 0x180046f90 | RehashByteKeyedMap | src/missing/MissingRound2Group3.cpp:187 | 0x18005e1d0 | 110 | 109 | 0.6484 |
| 0x18004d830 | mg5_DispatchPendingTasks | src/missing/MissingRound2Group2.cpp:351 | 0x1800653f0 | 102 | 106 | 0.7404 |
| 0x180046050 | mg5_WritePrefixedInt | src/missing/MissingRound2Group2.cpp:234 | 0x18005d3e0 | 76 | 76 | 0.7237 |
| 0x18002d340 | src/UiBridge.cpp:51 | src/missing/MissingRound2Group1.cpp:12 claims `anonymous-namespace'::JsCatalogDone -> src/UiBridge.cpp:51 | 0x18002cc30 | 67 | 52 | 0.6218 |
| 0x18004dfa0 | InitFormatArgStore | src/missing/MissingGroup3.cpp:143 | 0x180065de0 | 67 | 67 | 0.7761 |
| 0x180054d70 | ShutdownCaptionStore | src/missing/MissingRound2Group1.cpp:66 | 0x18006db00 | 51 | 28 | 0.5063 |
| 0x180047470 | AppendRangeToIterator | src/missing/MissingRound2Group3.cpp:242 | 0x18005e6e0 | 44 | 38 | 0.5610 |
| 0x180045980 | ReleaseSharedPtrControl | src/missing/MissingGroup3.cpp:365 | 0x18005cb80 | 41 | 40 | 0.6420 |
| 0x18003a630 | StringAppendRange | src/missing/MissingRound2Group3.cpp:77 | 0x1800241f0 | 40 | 40 | 0.7000 |
| 0x180040890 | src/missing/MissingGroup2.cpp:41 | src/missing/MissingRound2Group1.cpp:20 claims mg2_FillWords -> src/missing/MissingGroup2.cpp:41 | 0x180057640 | 38 | 39 | 0.7532 |
| 0x180023c10 | DestroyArray16 | src/missing/MissingRound2Group3.cpp:253 | 0x18004b5a0 | 25 | 31 | 0.7500 |
| 0x18002dda0 | DestroyFunctionArray | src/missing/MissingGroup3.cpp:397 | 0x18002c990 | 25 | 20 | 0.5333 |
| 0x18002c710 | AllocateVector16 | src/missing/MissingGroup3.cpp:134 | 0x180048270 | 20 | 14 | 0.6471 |
| 0x180010aa0 | DestroyOneFunction | src/missing/MissingRound2Group3.cpp:284 | 0x1800224f0 | 14 | 21 | 0.6286 |
| 0x180052f30 | ClearStateFlag8 | src/missing/MissingGroup3.cpp:422 | 0x18006c480 | 13 | 13 | 0.8462 |
| 0x180052f70 | CleanupFlaggedString_180052f70 | src/missing/MissingGroup1.cpp:91 | 0x18006c530 | 13 | 13 | 0.9231 |
| 0x180053120 | ClearFlag1DestroyString | src/missing/MissingRound2Group3.cpp:292 | 0x18006a190 | 13 | 13 | 0.9231 |
| 0x180053420 | ClearStateFlag2 | src/missing/MissingGroup3.cpp:433 | 0x18006c5ec | 13 | 13 | 0.9231 |
| 0x180001100 | mg5_InitActiveFalseGlobal | src/missing/MissingRound2Group2.cpp:127 | 0x18006db80 | 12 | 6 | 0.5556 |
| 0x1800236f0 | ReleaseInterface | src/missing/MissingRound2Group3.cpp:304 | 0x180021820 | 12 | 12 | 0.8333 |
| 0x18002bea0 | src/Presentation.cpp:115 | src/missing/MissingRound2Group1.cpp:13 claims `anonymous-namespace'::QueuePresentation -> src/Presentation.cpp:115 | 0x180032b70 | 11 | 15 | 0.8462 |
| 0x180054d30 | ShutdownUiGlobals | src/missing/MissingGroup3.cpp:217 | 0x18006c300 | 10 | 8 | 0.5556 |
| 0x180023b60 | DestroyFunctionPair | src/missing/MissingRound2Group3.cpp:314 | 0x180028c40 | 9 | 9 | 0.8889 |
| 0x1800382b0 | GetGameSettingDword | src/missing/MissingGroup3.cpp:208 | 0x1800416c0 | 5 | 5 | 0.8000 |

