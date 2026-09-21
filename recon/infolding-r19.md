# PRESENT-INFOLDED classification of the MISSING bucket

Original: `/home/aviallon/Projects/SexLabPPrism-re/artifacts/SexLabPPrism.dll`  |  Rebuild: `/home/aviallon/Projects/SexLabPPrism-re/artifacts/rebuild/parity-r19missing4.dll`  |  harness: `tools/pairfix_infolding.py`

This is a measurement layer on top of `tools/match.py --identity`. It changes no pair: PRESENT-INFOLDED is NOT a match and never counted as one.

## Scoreboard before (real functions, merged .pdata fragments)

`991` real functions, `85119` instructions

| verdict | funcs | insn |
|---|---|---|
| BYTE-MATCH | 631 | 41456 |
| INSN-MATCH | 39 | 630 |
| RATIO | 255 | 38796 |
| MISSING | 66 | 4237 |

## MISSING split (after classification)

    MISSING classified: 66 MISSING real functions, 4237 instructions
      PRESENT-INFOLDED    7 funcs    1114 insn
      PRESENT-UNPAIRED    1 funcs      12 insn
      UNKNOWN            58 funcs    3111 insn

Total MISSING is preserved: 66 = 7 PRESENT-INFOLDED + 1 PRESENT-UNPAIRED + 58 UNKNOWN

## Evidence per function

| class | orig addr | insn | name | anchor | value | owner | owner paired to |
|---|---|---|---|---|---|---|---|
| PRESENT-INFOLDED | 0x1800261e0 | 326 | `anonymous-namespace'::ApplyPresentation | literal | void __cdecl `anonymous-namespace'::ApplyPresentation(void) | 0x180012400 | 0x18002c260 | ActionDispatch::HandleAction |
| PRESENT-INFOLDED | 0x1800250f0 | 302 |  | literal | window.slppState( | 0x18002f470 | 0x180024450 |  |
| PRESENT-INFOLDED | 0x180047b60 | 194 |  | callee | 0x18004fa40 | 0x18005ef10 | 0x1800282b0 |  |
| UNKNOWN | 0x180047140 | 167 |  |  |  |  |  |  |
| UNKNOWN | 0x180047e40 | 158 |  |  |  |  |  |  |
| UNKNOWN | 0x18004ce40 | 146 |  |  |  |  |  |  |
| UNKNOWN | 0x18003eb30 | 134 |  |  |  |  |  |  |
| UNKNOWN | 0x180022be0 | 127 |  |  |  |  |  |  |
| UNKNOWN | 0x18003e400 | 115 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18003e850 | 115 |  | callee | 0x1800466d0 | 0x1800568a0 | 0x18003f150 |  |
| UNKNOWN | 0x180022e60 | 112 |  |  |  |  |  |  |
| UNKNOWN | 0x180046f90 | 110 |  |  |  |  |  |  |
| UNKNOWN | 0x18000fef0 | 102 |  |  |  |  |  |  |
| UNKNOWN | 0x18004d830 | 102 |  |  |  |  |  |  |
| UNKNOWN | 0x18001e000 | 100 |  |  |  |  |  |  |
| UNKNOWN | 0x180013f20 | 99 |  |  |  |  |  |  |
| UNKNOWN | 0x180044510 | 88 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18002cfd0 | 85 | `anonymous-namespace'::DispatchAction::<lambda_1 | literal | SLPPPrism_Action | 0x180012400 | 0x18002c260 | ActionDispatch::HandleAction |
| UNKNOWN | 0x180007dd0 | 82 |  |  |  |  |  |  |
| UNKNOWN | 0x1800358a0 | 82 |  |  |  |  |  |  |
| UNKNOWN | 0x180046050 | 76 |  |  |  |  |  |  |
| UNKNOWN | 0x18003d600 | 70 |  |  |  |  |  |  |
| UNKNOWN | 0x180044650 | 68 |  |  |  |  |  |  |
| UNKNOWN | 0x18002d340 | 67 |  |  |  |  |  |  |
| UNKNOWN | 0x18004dfa0 | 67 |  |  |  |  |  |  |
| UNKNOWN | 0x180010820 | 66 |  |  |  |  |  |  |
| UNKNOWN | 0x180044ab0 | 58 |  |  |  |  |  |  |
| UNKNOWN | 0x180010b20 | 55 |  |  |  |  |  |  |
| UNKNOWN | 0x18002e0a0 | 51 |  |  |  |  |  |  |
| UNKNOWN | 0x180054d70 | 51 |  |  |  |  |  |  |
| PRESENT-INFOLDED | 0x18002e000 | 47 |  | callee | 0x18002dd10 | 0x180012b20 | 0x18002c460 |  |
| PRESENT-INFOLDED | 0x180037fe0 | 45 | SKSE::MessagingInterface::RegisterListener | literal | Failed to register messaging listener for {} | 0x1800203b0 | 0x18002e250 | SKSEPlugin_Load |
| UNKNOWN | 0x18003e380 | 44 |  |  |  |  |  |  |
| UNKNOWN | 0x180047470 | 44 |  |  |  |  |  |  |
| UNKNOWN | 0x180006290 | 42 |  |  |  |  |  |  |
| UNKNOWN | 0x180045980 | 41 |  |  |  |  |  |  |
| UNKNOWN | 0x18003a630 | 40 |  |  |  |  |  |  |
| UNKNOWN | 0x180040890 | 38 |  |  |  |  |  |  |
| UNKNOWN | 0x1800483b0 | 35 |  |  |  |  |  |  |
| UNKNOWN | 0x18004da00 | 35 |  |  |  |  |  |  |
| UNKNOWN | 0x18003a370 | 34 |  |  |  |  |  |  |
| UNKNOWN | 0x180008a30 | 33 |  |  |  |  |  |  |
| UNKNOWN | 0x180023c10 | 25 |  |  |  |  |  |  |
| UNKNOWN | 0x18002dda0 | 25 |  |  |  |  |  |  |
| UNKNOWN | 0x180045d70 | 24 |  |  |  |  |  |  |
| UNKNOWN | 0x180037490 | 21 |  |  |  |  |  |  |
| UNKNOWN | 0x180037ae0 | 21 |  |  |  |  |  |  |
| UNKNOWN | 0x1800390b0 | 21 |  |  |  |  |  |  |
| UNKNOWN | 0x180039120 | 21 |  |  |  |  |  |  |
| UNKNOWN | 0x18002c710 | 20 |  |  |  |  |  |  |
| UNKNOWN | 0x1800374f0 | 20 |  |  |  |  |  |  |
| UNKNOWN | 0x180055090 | 20 |  |  |  |  |  |  |
| UNKNOWN | 0x180016d20 | 17 |  |  |  |  |  |  |
| UNKNOWN | 0x18002c9d0 | 16 |  |  |  |  |  |  |
| UNKNOWN | 0x180010aa0 | 14 |  |  |  |  |  |  |
| UNKNOWN | 0x180052f30 | 13 |  |  |  |  |  |  |
| UNKNOWN | 0x180052f70 | 13 |  |  |  |  |  |  |
| UNKNOWN | 0x180053120 | 13 |  |  |  |  |  |  |
| UNKNOWN | 0x180053420 | 13 |  |  |  |  |  |  |
| PRESENT-UNPAIRED | 0x180001100 | 12 |  | literal | {"active":false} | 0x180025f40 |  |  |
| UNKNOWN | 0x1800236f0 | 12 |  |  |  |  |  |  |
| UNKNOWN | 0x18002bea0 | 11 |  |  |  |  |  |  |
| UNKNOWN | 0x180054d30 | 10 |  |  |  |  |  |  |
| UNKNOWN | 0x180023b60 | 9 |  |  |  |  |  |  |
| UNKNOWN | 0x1800257d0 | 8 |  |  |  |  |  |  |
| UNKNOWN | 0x1800382b0 | 5 |  |  |  |  |  |  |
