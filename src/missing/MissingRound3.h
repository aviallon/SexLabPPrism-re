// src/missing/MissingRound3.h
// Owner: agent `missing7`, branch grind/missing7 (round 3 of the missing drive).
// Declares the matching-decompilation force-link anchor for the residue bodies
// in MissingRound3.cpp — the MISSING real (unwind-entry) functions that had NO
// declaration at all in recon/declared-mappings.csv.
//
// Call ForceLink_Missing7() once from src/main.cpp (a matching-decomp
// force-link, NOT behaviour): the bodies are unreferenced otherwise and
// /Gy + /OPT:REF would strip every one of them.
#pragma once

extern "C" void ForceLink_Missing7();
