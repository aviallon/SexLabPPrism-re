#pragma once

// MissingRound2Group3 — reconstructed bodies for the 21 MISSING real functions
// with group == "g3" in build/recon/missing-full.csv (~1,257 instructions).
//
// Decompiles: build/recon/decompiled-g3r2/0x<addr>_guess.c (Ghidra 12.1.2,
// cached project build/recon/ghidra-proj, script tools/ghidra/PrismExport.java).
//
// These are the functions the parity metric currently reports MISSING because
// nothing in our tree emits a counterpart. They are kept alive by the force-link
// table in MissingRound2Group3.cpp; call ForceLink_MissingRound2Group3() once
// from src/main.cpp (a matching-decomp force-link, NOT behaviour).
//
// NOTE: this is a different allocation from src/missing/MissingGroup3.cpp. Both
// exist; the group assignment in missing-full.csv changed between rounds, and
// 0x18002cfd0 / 0x180055090 / 0x180010aa0 appear in both worklists. Symbols are
// therefore in a separate namespace (MissingRound2Group3) to avoid ODR clashes,
// and 0x18002cfd0 is deliberately NOT reimplemented here.

namespace MissingRound2Group3
{
	// No public API. The only entry point is the C force-link sink.
}

// Implemented below (see .cpp for the full address -> name map):
//   plugin/behavioural 0x180047e40  ParentPath
//   plugin/behavioural 0x18003a630  StringAppendRange
//   plugin/behavioural 0x18002e000  GrowBSTArray
//   plugin/behavioural 0x18004da00  SetRegistryLevel
//   plugin/behavioural 0x180039120  LookupIdGlobal
//   library helpers    0x180047140  RehashStringMap
//                      0x18003eb30  WriteAlignedField
//                      0x180046f90  RehashByteKeyedMap
//                      0x18001e000  ThreadJoin
//                      0x1800358a0  FileBufOpen
//                      0x180010820  DestroyFunctionVector
//                      0x18002e0a0  LookupAndCallIdFunction2
//                      0x180047470  AppendRangeToIterator
//                      0x180023c10  DestroyArray16
//                      0x180045d70  FlushBufferToTarget
//                      0x180055090  LookupAndCallIdFunction
//                      0x180010aa0  DestroyOneFunction
//                      0x180053120  ClearFlag1DestroyString
//                      0x1800236f0  ReleaseInterface
//                      0x180023b60  DestroyFunctionPair
//
//   PRESENT-BUT-UNPAIRED: 0x18002cfd0 (`anonymous-namespace'::DispatchAction::
//     <lambda_1>::operator()). The body already ships in src/ActionDispatch.cpp
//     (HandleAction: logs "UI action dispatched to Papyrus: {}" and sends the
//     SLPPPrism_Action mod event). It is intentionally not duplicated here.
extern "C" void ForceLink_MissingRound2Group3();
