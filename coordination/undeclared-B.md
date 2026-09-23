## 2026-09-23 — undeclared-B: middle third of the 171 REAL MISSING

- Branch `grind/undeclared-b`, fresh from `main @ a055c33`. Worktree `wt/undeclared-b`.
- Slice = middle 57 of the 171 REAL MISSING (unwind-entry) originals, sorted by
  address ascending, measured on `artifacts/rebuild/parity-zero.dll` vs
  `artifacts/SexLabPPrism.dll` (md5 61c222d7...; report
  `recon/parity-runs/zero/per-function-declared-map.json`). Range
  `0x180037840 .. 0x180046910`. First/last thirds untouched.
- 7 structure-faithful bodies in `src/missing/MissingRoundNotDeclaredB.cpp`
  (the REL::ID/id2offset indirection family). Each declared in the
  `extract_declared.py` format; force-linked via XOR table
  `ForceLink_MissingRoundNotDeclaredB()`.
- 36 rows are library-provided (spdlog formatters, MSVC STL, CLNG) — no body,
  providing file:line in the catalogue. 11 are unnamed/large/degraded residue.
- **NEEDS FROM MAIN:** add `#include "missing/MissingRoundNotDeclaredB.h"` and a
  `ForceLink_MissingRoundNotDeclaredB();` call to `SKSEPlugin_Load` in
  `src/main.cpp` (the orchestrator owns main.cpp). Without it /Gy+/OPT:REF
  strips all 7 bodies.
- Proof: object `llvm-nm` lists all 7 + anchor; a lld-link image built with
  `/OPT:REF /OPT:ICF` keeps all 7 at DISTINCT addresses
  (0x140001060/10c0/1120/1180/11e0/1260/1360). Details in my handoff.
