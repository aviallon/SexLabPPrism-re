# PRESENT-INFOLDED classification of the MISSING bucket

Original: `/home/aviallon/Projects/SexLabPPrism-re/artifacts/SexLabPPrism.dll`  |  Rebuild: `/home/aviallon/Projects/SexLabPPrism-re/artifacts/rebuild/parity-r14revert.dll`  |  harness: `tools/pairfix_infolding.py`

This is a measurement layer on top of `tools/match.py --identity`. It changes no pair: PRESENT-INFOLDED is NOT a match and never counted as one.

## Scoreboard before (real functions, merged .pdata fragments)

`911` real functions, `85148` instructions

| verdict | funcs | insn |
|---|---|---|
| BYTE-MATCH | 546 | 38828 |
| INSN-MATCH | 37 | 601 |
| RATIO | 240 | 39233 |
| MISSING | 88 | 6486 |

## MISSING split (after classification)

    MISSING classified: 88 MISSING real functions, 6486 instructions
      PRESENT-INFOLDED   11 funcs    1271 insn
      ABSENT              2 funcs     181 insn
      UNKNOWN            75 funcs    5034 insn

Total MISSING is preserved: 88 = 11 PRESENT-INFOLDED + 2 ABSENT + 75 UNKNOWN

## Evidence per function

| class | orig addr | insn | name | anchor | value | owner | owner paired to |
|---|---|---|---|---|---|---|---|
| UNKNOWN | 0x180013a20 | 349 |  |  |  |  |  |  |
| UNKNOWN | 0x1800464d0 | 349 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x1800250f0 | 302 |  | literal | window.slppState( | 0x180028b90 | 0x180024450 |  |
| PRESENT-INFOLDED | 0x18003aca0 | 215 |  | callee | 0x180010f80 | 0x180006570 | 0x180006740 |  |
| UNKNOWN | 0x180047140 | 207 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x180047b60 | 194 |  | callee | 0x18004fa40 | 0x1800580b0 | 0x1800282b0 |  |
| UNKNOWN | 0x18003af80 | 173 |  |  |  |  |  |  |
| ABSENT | 0x180025ce0 | 169 |  | literal | \u{:04X}\u{:04X} |  |  |  |
| UNKNOWN | 0x18003d160 | 163 |  |  |  |  |  |  |
| UNKNOWN | 0x180047e40 | 158 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18002c460 | 153 |  | callee | 0x180038b00 | 0x180012850 | 0x18002c0d0 | `anonymous-namespace'::RequestSearchInpu |
| UNKNOWN | 0x18004ce40 | 146 |  |  |  |  |  |  |
| UNKNOWN | 0x18003eb30 | 134 |  |  |  |  |  |  |
| UNKNOWN | 0x18002dad0 | 129 |  |  |  |  |  |  |
| UNKNOWN | 0x180044ca0 | 126 | .?AVlogger@spdlog@@::vtable[0] |  |  |  |  |  |
| UNKNOWN | 0x18003a830 | 123 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18004a450 | 119 | .?AV?$elapsed_formatter@Vscoped_padder@details@s | rtti | .?AV?$elapsed_formatter@Vscoped_padder@details@spdlog@@V?... | 0x18005ae90 | 0x18004a2f0 | .?AV?$elapsed_formatter@Vscoped_padder@d |
| UNKNOWN | 0x180022570 | 117 |  |  |  |  |  |  |
| UNKNOWN | 0x180023230 | 117 |  |  |  |  |  |  |
| UNKNOWN | 0x18003e400 | 115 |  |  |  |  |  |  |
| UNKNOWN | 0x18003e850 | 115 |  |  |  |  |  |  |
| UNKNOWN | 0x180046f90 | 110 |  |  |  |  |  |  |
| UNKNOWN | 0x18000fef0 | 102 |  |  |  |  |  |  |
| UNKNOWN | 0x18004d830 | 102 |  |  |  |  |  |  |
| UNKNOWN | 0x18001e000 | 100 |  |  |  |  |  |  |
| UNKNOWN | 0x180013f20 | 99 |  |  |  |  |  |  |
| UNKNOWN | 0x180044510 | 88 |  |  |  |  |  |  |
| UNKNOWN | 0x180044ab0 | 88 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18002cfd0 | 85 | `anonymous-namespace'::DispatchAction::<lambda_1 | literal | SLPPPrism_Action | 0x180012400 | 0x18002c260 | ActionDispatch::HandleAction |
| UNKNOWN | 0x180046db0 | 84 |  |  |  |  |  |  |
| UNKNOWN | 0x180007dd0 | 82 |  |  |  |  |  |  |
| UNKNOWN | 0x1800358a0 | 82 |  |  |  |  |  |  |
| UNKNOWN | 0x180046050 | 76 |  |  |  |  |  |  |
| UNKNOWN | 0x18005156c | 72 |  |  |  |  |  |  |
| UNKNOWN | 0x180044650 | 68 |  |  |  |  |  |  |
| UNKNOWN | 0x18002d340 | 67 |  |  |  |  |  |  |
| UNKNOWN | 0x18004dfa0 | 67 |  |  |  |  |  |  |
| UNKNOWN | 0x180010820 | 66 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18004e670 | 57 | .?AV?$wincolor_sink@Uconsole_mutex@details@spdlo | callee | 0x180045bb0 | 0x18005f700 | 0x18004e4c0 | .?AV?$basic_file_sink@Vmutex@std@@@sinks |
| UNKNOWN | 0x180010b20 | 55 |  |  |  |  |  |  |
| UNKNOWN | 0x18002e0a0 | 51 |  |  |  |  |  |  |
| UNKNOWN | 0x180054d70 | 51 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18002e000 | 47 |  | callee | 0x18002dd10 | 0x180012850 | 0x18002c0d0 | `anonymous-namespace'::RequestSearchInpu |
| UNKNOWN | 0x180047500 | 47 |  |  |  |  |  |  |
| UNKNOWN | 0x18004fb6c | 45 |  |  |  |  |  |  |
| UNKNOWN | 0x18003e380 | 44 |  |  |  |  |  |  |
| UNKNOWN | 0x180047470 | 44 |  |  |  |  |  |  |
| UNKNOWN | 0x180006290 | 42 |  |  |  |  |  |  |
| UNKNOWN | 0x180045980 | 41 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18003a630 | 40 |  | callee | 0x18003cd70 | 0x18002cec0 | 0x180028190 | PrismaUI::InvokeJs |
| UNKNOWN | 0x180046f00 | 39 |  |  |  |  |  |  |
| UNKNOWN | 0x180040890 | 38 |  |  |  |  |  |  |
| UNKNOWN | 0x18004da00 | 35 |  |  |  |  |  |  |
| UNKNOWN | 0x180037a40 | 34 |  |  |  |  |  |  |
| UNKNOWN | 0x18003a370 | 34 |  |  |  |  |  |  |
| UNKNOWN | 0x180054e50 | 34 |  |  |  |  |  |  |
| UNKNOWN | 0x180008a30 | 33 |  |  |  |  |  |  |
| UNKNOWN | 0x180018cc0 | 32 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18001bbb0 | 30 |  | callee | 0x18001c970 | 0x18001b7a0 | 0x18001d8b0 | REL::IDDatabase::id2offset |
| UNKNOWN | 0x1800140c0 | 29 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18004d0f0 | 29 |  | callee | 0x180045230 | 0x18005dee0 | 0x18004d060 |  |
| UNKNOWN | 0x180023c10 | 25 |  |  |  |  |  |  |
| UNKNOWN | 0x18002dda0 | 25 |  |  |  |  |  |  |
| UNKNOWN | 0x180045d70 | 24 |  |  |  |  |  |  |
| UNKNOWN | 0x1800107a0 | 22 |  |  |  |  |  |  |
| UNKNOWN | 0x1800163a0 | 22 |  |  |  |  |  |  |
| UNKNOWN | 0x180037490 | 21 |  |  |  |  |  |  |
| UNKNOWN | 0x180037ae0 | 21 |  |  |  |  |  |  |
| UNKNOWN | 0x1800390b0 | 21 |  |  |  |  |  |  |
| UNKNOWN | 0x180039120 | 21 |  |  |  |  |  |  |
| UNKNOWN | 0x18002c710 | 20 |  |  |  |  |  |  |
| UNKNOWN | 0x180055090 | 20 |  |  |  |  |  |  |
| UNKNOWN | 0x180016d20 | 17 |  |  |  |  |  |  |
| UNKNOWN | 0x180016820 | 16 |  |  |  |  |  |  |
| UNKNOWN | 0x18002c9d0 | 16 |  |  |  |  |  |  |
| UNKNOWN | 0x18002cb60 | 16 |  |  |  |  |  |  |
| UNKNOWN | 0x180010aa0 | 14 |  |  |  |  |  |  |
| UNKNOWN | 0x180052f30 | 13 |  |  |  |  |  |  |
| UNKNOWN | 0x180052f70 | 13 |  |  |  |  |  |  |
| UNKNOWN | 0x180053120 | 13 |  |  |  |  |  |  |
| UNKNOWN | 0x180053420 | 13 |  |  |  |  |  |  |
| ABSENT | 0x180001100 | 12 |  | literal | {"active":false} |  |  |  |
| UNKNOWN | 0x1800236f0 | 12 |  |  |  |  |  |  |
| UNKNOWN | 0x18002bea0 | 11 |  |  |  |  |  |  |
| UNKNOWN | 0x1800382b0 | 10 |  |  |  |  |  |  |
| UNKNOWN | 0x180054d30 | 10 |  |  |  |  |  |  |
| UNKNOWN | 0x180023b60 | 9 |  |  |  |  |  |  |
| UNKNOWN | 0x180010cb0 | 7 |  |  |  |  |  |  |

### FOCUS 0x1800250f0
```json
{
  "orig_addr": 6442602736,
  "orig_insn": 302,
  "name": "",
  "class": "PRESENT-INFOLDED",
  "evidence": {
    "anchor": "literal",
    "value": "window.slppState(",
    "owner": 6442617744,
    "owner_paired_to": 6442599504,
    "owner_name": ""
  },
  "anchors": {
    "lits": 2,
    "unique_lits": 2,
    "callees": 7,
    "unique_callees": 0
  }
}
```
