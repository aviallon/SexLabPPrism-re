# anonns agent (grind/anonns) — round 9 anonymous-namespace layout

Forks `main` @ `40227ab`. Branch `grind/anonns` @ `991120e`, worktree `wt/bridge`.
Owns only `src/ActionDispatch.*`, `src/UiBridge.*`, `src/PrismaUI.*`.

## What changed

The original unity TU has exactly two unnamed namespaces: the global one
(`?A0xbb2e73b6`, covering DispatchAction, the Papyrus natives, PublishCatalogToUi,
PushState, PushCompatible, QueuePresentation, RequestSearchInput, InputSink,
MenuVisibilitySink) and FocusRecovery's nested one (`?A0xa6da2f39@FocusRecovery`).
Three of my files still had their own nested anonymous namespaces:

- `ActionDispatch.cpp`: `ActionOf/ArgOf` were in `ActionDispatch::anonymous-ns`.
- `UiBridge.cpp`: `QueueOnGameThread` was in `UiBridge::anonymous-ns`.
- `PrismaUI.cpp`: constants, `State`, `g_state`, `Slot<>`, the JS trampolines
  (`OnViewCreated`, `OnSlppReady/Log/CatalogRetry/Collapsed`) were in
  `PrismaUI::anonymous-ns`.

All of them now live in the file-scope global unnamed namespace (the
bridgeparams pattern already used by InputSink/MenuVisibilitySink). No behaviour,
signature or public-API change. The single semantic edit is
`namespace V = vtbl;` -> `namespace V = PrismaUI::vtbl;` (`vtbl` is declared
inside `namespace PrismaUI` in `PrismaUI_vtbl.h`).

## Evidence (clang-cl local, not the oracle)

Compiled the 11-file unity TU (unitybuild's farm order) with clang-cl 21 and
inspected mangled names:

| build | distinct `?A0x` scope forms in the unity object |
|---|---|
| `origin/main` (`40227ab`, before) | `?A0x2A4FF614@FocusRecovery`, `?A0x2A4FF614@PrismaUI` |
| `grind/anonns` (`991120e`, after) | `?A0xFC0F7AE5@FocusRecovery` only |

No `@PrismaUI` / `@ActionDispatch` / `@UiBridge` / `@InputSink` /
`@MenuVisibilitySink` scope form remains. clang reuses one hex per TU and adds an
`@Scope` suffix for a nested unnamed namespace, so the task's exact
`grep -o '?A0x[0-9a-f]*'` count is 1 -> 1 on clang objects; the scope suffix is
the clang-visible equivalent.

## MSVC prediction (for the CI unity build)

MSVC gives each distinct unnamed namespace a distinct hex. The unified TU should
therefore go from 3 distinct `?A0x` hexes (global + PrismaUI-nested +
FocusRecovery-nested) to **2** (global + FocusRecovery-nested). Whether the
global hex is the original's `bb2e73b6` still depends on unitybuild's include
order and MSVC's path-hash input — layout alone cannot force that value.

Joint verification is with `unitybuild` on their farm; not queued by me.