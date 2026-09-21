#pragma once

// MissingRound2Group1 — round-2 (relaunch) reconstructions for the 20 MISSING
// real functions with group == "g1" in build/recon/missing-full.csv.
//
// Owner: agent `missing4`, branch grind/missing4, file src/missing/MissingRound2Group1.*
//
// Decompiles: build/recon/decompiled-g1r2/0x<addr>_*.c (Ghidra 12.1.2, cached
// project build/recon/ghidra-proj, script tools/ghidra/PrismExport.java, 20/20
// clean).
//
// Files with an existing body elsewhere in src/ are NOT duplicated here; they
// are reported PRESENT-BUT-UNPAIRED with the file:line evidence (see the .cpp
// header). This file carries bodies only for the addresses that had none.
//
// The bodies below are matching-decompilation artefacts, NOT plugin behaviour:
// they are kept out of /OPT:REF's reach by the force-link table and are never
// reached on a live runtime path. Call ForceLink_Missing4Group1() once from
// src/main.cpp.

namespace MissingRound2Group1
{
	// No public API. The only entry point is the C force-link sink.
}

// Implemented below (see the .cpp for the per-address provenance):
//   0x180006290 (library) RefBlockRelease── refcounted thread control block
//   0x180013f20 (plugin)  ScheduleDelayedTask — timed task queue push
//   0x180023230 (library) ConcatStrings — std::string operator+
//   0x18002c9d0 (library) CatalogAppendFunction — _Func_impl copy ctor
//   0x1800390b0 (plugin)  IdLookup_615a0 — REL::IDDatabase lookup
//   0x1800374f0 (plugin)  IdLookup_7e3b3 — REL::IDDatabase lookup
//   0x18003e6e0 (library) FormatAlignedField — std::format fill/align writer
//   0x180044ab0 (library) MoveBlockStore — deque-style block move ctor
extern "C" void ForceLink_Missing4Group1();