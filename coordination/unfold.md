# unfold — Class-1 residue + infolding boundaries (grind/unfold)

## 2026-09-22 — grind/unfold @ 7e8a4b2 (src); see tip commit

**Branch:** `grind/unfold` (worktree `wt/revive`), branched fresh from `main` @ `9ed61fa`.

**Hook lines needed in src/main.cpp: 0.** The force-link entry point
`ForceLink_UnfoldUiBridge` (src/UiBridge.cpp) keeps itself alive with
`#pragma comment(linker, "/include:ForceLink_UnfoldUiBridge")`, which routes to
a linker `/INCLUDE` directive. Verified in a real `lld-link /OPT:REF /OPT:ICF`
link: `ForceLink_UnfoldUiBridge`, `?JsCatalogReset@…`, `?JsCatalogDone@…` are all
listed in the /MAP output. If main prefers an explicit call, one line is enough
and the pragma can stay (it is idempotent).

### Class 1 — declared bodies with no boundary / in no force table

| orig | body | mechanism | link-level evidence (/MAP, local clang-cl /OPT:REF build) |
|---|---|---|---|
| 0x1800261e0 | `Presentation::ApplyPresentation` | `__declspec(noinline)` added. It is referenced (InputSink F3/F4, PushStateBody, SetSearchQuery task lambda), so it is not stripped — but the small body was being folded into every caller, so no standalone boundary existed. Not unreferenced, so no force table. | `?ApplyPresentation@Presentation@@YAXXZ` present |
| 0x18002bea0 | anon-ns `QueuePresentation` + `Presentation::QueuePresentation` shim | already `noinline` on the anon body; added `noinline` to the public shim so the shim cannot swallow the anchored body. Referenced from InputSink. | `t ?QueuePresentation@?A0x891D7885@@YAXXZ` + `T ?QueuePresentation@Presentation@@YAXXZ` present |
| 0x18002d340 | anon-ns `JsCatalogDone` (and 0x18002d170 `JsCatalogReset`) | genuinely unreferenced: `UiBridge::PushCatalog` is dead and `CatalogNatives`/`RetryPublish` call `InvokeJs` directly. Added a whole-table XOR force table `kForceUnfoldUiBridge` in src/UiBridge.cpp + `ForceLink_UnfoldUiBridge()`. | `?JsCatalogReset@?A0xDB056EC0@@YAXH@Z`, `?JsCatalogDone@?A0xDB056EC0@@YAX_J@Z`, `ForceLink_UnfoldUiBridge` all present |
| 0x180016d20 | `FocusRecovery::anon::CloseConsolePulse` | already `noinline` and reachable on a live chain (InputSink → Begin task → Run → CheckUnfocus/VerifyCleanup → StartConsolePulse → CloseConsolePulse). No change needed; no force table. | `t ?CloseConsolePulse@?A0xECFB7B90@FocusRecovery@@YAX_K@Z` present |

### Class 2 — PRESENT-INFOLDED rows (recon/infolding-r19.json, 7 funcs / 1,114 insns)

Direction key: original keeps two functions separate, our build folds them →
`noinline` belongs on OUR contained function.

| orig | insn | owner (r19) | direction / action |
|---|---|---|---|
| 0x1800261e0 ApplyPresentation | 326 | 0x180012400 (paired 0x18002c260 SendAction) | OUR fold → `noinline` on `Presentation::ApplyPresentation`. **Applied.** (The r19 "literal" anchor is the fuzzy `void __cdecl `anonymous-namespace'::` prefix, so it is weak; the boundary is real regardless.) |
| 0x1800250f0 window.slppState( | 302 | 0x18002f470 | Already has a standalone boundary: `MissingRound4.cpp::PublishVitalsState` (`noinline`, force-linked by `ForceLink_MissingRound4`, owned by zero). No action. |
| 0x180047b60 | 194 | 0x18005ef10 (library) | OUR build's owner is a **spdlog/library** function, not project source. The fold is inside CLNG/spdlog and cannot be moved from this repo's `src/` without touching vendored library code. NOT FIXABLE HERE. |
| 0x18003e850 | 115 | 0x1800568a0 (library) | Same: owner is a library (spdlog) function. NOT FIXABLE HERE. |
| 0x18002cfd0 DispatchAction::<lambda_1> | 85 | 0x180012400 (HandleAction) | Already has a standalone boundary: `MissingGroup3::DispatchActionLambda` (`noinline`, in `kForce3`, force-linked by `ForceLink_MissingGroup3`). No action. |
| 0x18002e000 | 47 | 0x180012b20 (paired 0x18002c460) | Weak single-callee anchor (`RE::BSTArrayHeapAllocator::allocate`): that callee is also the anchor for the 153-insn owner 0x18002c460, so the classifier cannot show the 47-insn body is folded. No distinct source function identified as the contained one; needs the linker-map/declared binding to decide. NOT DONE. |
| 0x180037fe0 RegisterListener | 45 | 0x1800203b0 SKSEPlugin_Load | OUR fold, but the contained function is **CLNG's inline `SKSE::MessagingInterface::RegisterListener`** (src/Lifecycle.cpp call site). Forcing it out-of-line would touch Lifecycle.cpp (bridge-owned) or CLNG. NOT DONE HERE. |

### Verification method

`/tmp` had been wiped; re-created the xwin sysroot from the existing
`/tmp/xwin-sysroot` (it survived) and re-cloned CLNG to `/tmp/CommonLibSSE-NG`
(shallow + `extern/openvr`). Built with `tools/local-build.sh` flags
(clang-cl, `/Gy /OPT:REF /OPT:ICF`, no `/GL`). Evidence is the link MAP, which is
strictly stronger than the object check: it lists symbols in the final image
after `/OPT:REF`. MSVC has no `__builtin_*`; none used.