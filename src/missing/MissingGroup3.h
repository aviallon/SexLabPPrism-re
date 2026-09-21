#pragma once

// MissingGroup3 — round-3 matching-decomp reconstructions for the 32 MISSING
// real functions assigned to group g3 (build/recon/missing-full.csv).
//
// The force-link entry point only exists to keep the bodies out of /OPT:REF's
// reach. It is a matching artifact, not plugin behaviour: nothing in the plugin
// calls it. See MissingGroup3.cpp for the per-address provenance of every body.

namespace MissingGroup3
{
}  // namespace MissingGroup3

// Called once from src/main.cpp. Without it /Gy + /OPT:REF strip every body.
extern "C" void ForceLink_MissingGroup3();