// src/missing/MissingRound2Group2.h
// Owner: agent `missing5`, branch grind/missing5 (relaunch of the g2 group).
// Declares the matching-decompilation force-link anchor for the second-round
// g2 bodies in MissingRound2Group2.cpp.
//
// Call ForceLink_Missing5Group2() once from src/main.cpp (a matching-decomp
// force-link, NOT behaviour): the bodies are unreferenced otherwise and
// /Gy + /OPT:REF would strip every one of them.
#pragma once

extern "C" void ForceLink_Missing5Group2();
