# catalogbody agent (grind/catalogbody) — round 8

Branch `grind/catalogbody` @ `wt/matchratio2`. Owns `src/Catalog.*`,
`src/SceneState.*`, and now `src/Papyrus/CatalogNatives.*` (the 7 catalogue
native bodies, split out of tasklambda's `src/Papyrus/Natives.cpp`).
Commit `04b2b2b`.

## 2026-09-21 — catalogue natives split out; real CatalogAppend shape

### Ownership split (decided with main + tasklambda)
The round-7 shaperefactor had moved all 13 native bodies into
`src/Papyrus/Natives.cpp`. That made `Papyrus_CatalogAppend` (my top-priority
body work) live in tasklambda's file. Main's resolution: create
`src/Papyrus/CatalogNatives.{h,cpp}`, move the 7 catalogue bodies there,
implement CatalogAppend there, and surgically delete the moved definitions from
Natives.cpp in ONE commit (internal-linkage bodies in two TUs would otherwise
not link-error — the old definitions would simply keep being registered).

### What changed
- `src/Papyrus/CatalogNatives.cpp/.h`: the 7 catalogue bodies in the global
  anonymous namespace; `Papyrus::Natives::RegisterCatalog(VM*)` registers them
  (an anonymous namespace cannot be named from another TU).
- `Catalog::Record` corrected from
  `{string id, string name, vector<string> tags, uint64 reserved, string package}`
  to the original's **four std::strings** `{id, name, tags, package}`.
  `tags` is ONE comma-separated string: `FUN_1800273a0` splits it inline with a
  memchr loop (record+0x40 size/cap at +0x50/+0x58, i.e. a std::string SBO, and
  record+0x60 is the package defaulted to `"Unsorted"`). The recon doc's
  "vector<string> tags" inference was wrong.
- `Papyrus_CatalogAppend` rewritten to the original's one-pass shape: `[` opened
  before the lock, each record built by copy, ONE `FormatRow` call per record
  (our stand-in for FUN_1800273a0), FNV-keyed `try_emplace` then record copied
  into the vector, `]` appended after the unlock, then a single SKSE task lambda
  instantiated **inside the native** (captures rows+loaded) so the
  `_Func_impl_no_alloc` wrapper is named after `Papyrus_CatalogAppend`.
- `Natives.cpp`: removed the 7 definitions, the catalogue state, `NowTicks`,
  `SplitTags`, `RowsJson`, `Catalog::Fnv1aHash::operator()`, `RetryPublish`; the
  7 catalog `RegisterFunction` lines became `RegisterCatalog(a_vm)`.

### Measured "before" (parity-r8, `tools/match.py --focus`)
| addr | verdict / ratio |
|---|---|
| 0x180029740 CatalogAppend | MISSING (r7 ratio 0.1371) |
| 0x18002a000 CatalogBegin | MISSING |
| 0x18002a410 CatalogPackage | RATIO 0.6257 first_div 15 |
| 0x18002a270 CatalogFinish | RATIO 0.8571 first_div 0 |
| 0x18002a6e0 CatalogPublish | MISSING |
| 0x18002a830 GetCatalogCount | MISSING |
| 0x18002a8a0 IsCatalogReady | 7-insn anomaly |

Note: the matcher reports 7-insn bodies at 0x18002a270/0x18002a6e0/0x18002a830
in the ORIGINAL, which contradicts the Ghidra decompilation (CatalogFinish 61
lines, CatalogPublish 50). There is an address/pin mismatch worth re-checking;
it is not caused by this change.

### Honest status
No BYTE-MATCH claim. The remaining CatalogAppend gap is that clang-cl cannot
reproduce the original's exact STL inlining order without the exact source; this
commit fixes the data model and control flow (record copies, single formatter
call, map+vector, in-body lambda dispatch) so the instruction mass moves toward
549. Compile check: CatalogNatives.cpp and Natives.cpp compile clean with
clang-cl (0 errors); full local link not run (45-min cap).
