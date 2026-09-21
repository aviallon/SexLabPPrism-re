# unitybuild agent (grind/unitybuild) — round 9 unity-TU layout

Forks `main` @ `6a55a88`. Branch `grind/unitybuild`, worktree `wt/libh3`.
Owns `xmake.lua`, `.github/workflows/build.yml`, `docs/`, `tools/`, and the
include farm in `src/main.cpp`. Full evidence: `recon/unity-build.md`.

## Summary

The original is a **unity build**, proven by the token grouping (not merely
suspected): every plugin `?A0x` literal carries one of exactly two tokens —
`?A0xbb2e73b6` (global anonymous namespace: DispatchAction, the five
`Papyrus_Catalog*`/`SetSearchQuery` natives, PublishCatalogToUi, PushState,
PushCompatible, QueuePresentation, RequestSearchInput, InputSink,
MenuVisibilitySink — all in *different* files in our tree) and
`?A0xa6da2f39@FocusRecovery` (FocusRecovery's internal anonymous namespace).
Our separate-TU r8 has four tokens. `_Func_impl_no_alloc`: original 28 (19
lambda), r8 21 (12 lambda).

## What I changed (mechanical, compiles clean)

- `src/main.cpp`: include farm — `#include "FocusRecovery.cpp"` first, then
  ActionDispatch, UiBridge, Presentation, PrismaUI, Lifecycle, InputSink,
  MenuVisibilitySink, SceneState, Catalog, `Papyrus/Natives.cpp`.
  `main.cpp`'s own top `namespace {}` stays first (global namespace first);
  `FocusRecovery.cpp` is first in the farm so its nested namespace is the
  second unnamed namespace of the TU.
- `xmake.lua`: `remove_files(...)` for those 11 files (shipping + parity both
  build the unity TU).
- `tools/local-build.sh`: compile only `src/main.cpp` (the loop used to compile
  every `src/*.cpp`, which would now double-define every symbol).

Local clang-cl 21 full compile of `src/main.cpp`: **0 errors**. Object shows
`RequestSearchInput@?A0x<unity>@@` and `CloseConsolePulse@?A0x<unity>@FocusRecovery`;
the old `?A0xd1b633de`/`?A0xf42b6800` are gone.

## BLOCKER / request to the owners of src/

After rebasing on `main` @ `40227ab`, `bridgeparams` has landed and
`InputSink`/`MenuVisibilitySink` are top-level, so their spurious
`@InputSink`/`@MenuVisibilitySink` tokens are gone from the unity object. The
unity object still has one spurious nested token, `@PrismaUI`, plus nested
anonymous namespaces in `ActionDispatch.cpp` and `UiBridge.cpp` that emit no
RTTI string but still count as extra unnamed namespaces. The original has only
TWO unnamed namespaces, so FocusRecovery cannot be the second one until those
are flattened. `grind/anonns` owns PrismaUI/ActionDispatch/UiBridge now.

Separately, `src/Papyrus/CatalogNatives.cpp` (split out by `catalogbody`)
cannot join the farm: its global anonymous namespace redefines `kClassName` and
`QuoteJson`, which `Natives.cpp` also defines -> compile error. It stays a
separate TU (its own `?A0x` token) until `catalogbody` shares those helpers via
a header. This is a second, independent reason one catalog token will not match
this round.

## CI prediction (do not queue until round-9 pair lands)

- The layout makes `?A0xbb2e73b6` *possible*; it lands **only if** the
  reshaping above removes the extra nested namespaces AND MSVC's hash input
  matches the original. MSVC hashes a per-TU path (LLVM D50877: absolute path;
  clang uses the path as passed). The original has no PDB path / drive-letter
  string, so if MSVC used the absolute path the value is unrecoverable by
  layout alone. This build decides it.
- `_Func_impl_no_alloc` count: unity relabels existing wrappers; it does not
  add the 7 missing ones (that is source shape, tasklambda/bridgeparams).
- No-regression baseline is the CORRECTED inventory (main 40227ab merges
  contiguous .pdata fragments): r8 = 535/911 real BYTE (58.7%),
  instruction-weighted 44.7%, MISSING 95; r9pair = 549/911 (60.3%), 46.0%,
  MISSING 86. Expect no regression below those (pairing is structural,
  `tools/match.py::pair`).

Request: queue ONE parity build (`lto_scope=off`) after the round-9 pair is in
`main`; I will rebase `grind/unitybuild` and ask again if the shapes moved.