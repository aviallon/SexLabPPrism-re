// src/missing/MissingRoundNotDeclaredB.h
// Owner: agent `undeclared-B`, branch grind/undeclared-b.
//
// Matching-decompilation force-link anchor for the bodies reconstructed in
// src/missing/MissingRoundNotDeclaredB.cpp.
//
// The parity build compiles with /Gy and links with /OPT:REF, so a function
// nothing references is stripped from the DLL and can never pair with its
// original address. Call ForceLink_MissingRoundNotDeclaredB() once from a live
// path (src/main.cpp owns that call - the orchestrator adds it). This is a
// matching-decomp artefact, NOT behaviour: nothing on the live runtime path may
// call any of the reconstructed stubs.
#pragma once

extern "C" void ForceLink_MissingRoundNotDeclaredB();