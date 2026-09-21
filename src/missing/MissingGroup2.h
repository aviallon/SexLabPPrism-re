#pragma once

// MissingGroup2 — reconstructed bodies for the 32 MISSING functions with
// group == "g2" in build/recon/missing-full.csv (~2,592 instructions).
//
// Decompiles: build/recon/decompiled-g2/0x<addr>_anon.c (Ghidra 12.1.2, cached
// project build/recon/ghidra-proj, script tools/ghidra/PrismExport.java).
//
// These are the functions the parity metric currently reports MISSING because
// nothing in our tree emits a counterpart. They are kept alive by the
// force-link table in MissingGroup2.cpp; call ForceLink_MissingGroup2() once
// from src/main.cpp (a matching-decomp force-link, NOT behaviour).

namespace MissingGroup2
{
	// No public API. The only entry point is the C force-link sink.
}

// Implemented below (see .cpp for the address -> name map):
//   plugin/behavioural 0x180014140  SendMenuMessage (lambda)
//   plugin/behavioural 0x180022340  (GUESS) PublishUiStatePacked
//   plugin/behavioural 0x1800255c0  RequestSearchInput lambda (FocusMenu path)
//   library helpers    see kForce2 in the .cpp
extern "C" void ForceLink_MissingGroup2();
