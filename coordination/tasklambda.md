# tasklambda agent (grind/tasklambda) — native TaskInterface-lambda dispatch

Worktree `wt/shaperefactor`, branch `grind/tasklambda`, owns `src/Papyrus/*`
(catalog natives reassigned to catalogbody — see below).

## Why: the original dispatches its own TaskInterface lambdas inline

Original `_Func_impl_no_alloc` wrapper types attached to native bodies
(recon/decompiled/0x18002*.c, grep `Func_impl`):

| native | addr | wrapper type | captured |
|---|---|---|---|
| PublishSceneState | 0x18002ae90 | ``anonymous-namespace'::PushState(void)::lambda_1`` | nothing (reads state-JSON global) |
| PublishCompatible | 0x18002a9a0 | ``anonymous-namespace'::PushCompatible(void)::lambda_1`` | nothing (reads compatible-JSON global) |
| CatalogBegin | 0x18002a000 | ``Papyrus_CatalogBegin(...)::lambda_1`` | the `int total` |
| CatalogAppend | 0x180029740 | ``Papyrus_CatalogAppend(...)::lambda_1`` | rows JSON string + loaded + total |
| CatalogFinish | 0x18002a270 | ``Papyrus_CatalogFinish(void)::lambda_1`` | the record count |
| CatalogPublish | 0x18002a6e0 | ``Papyrus_CatalogPublish(void)::lambda_1`` | the record count |
| SetSearchQuery | 0x18002bd20 | ``Papyrus_SetSearchQuery(...)::lambda_1`` | the query std::string (moved) |

Log (0x18002a8e0), BeginSceneSession (0x180029630), IsFreeCameraActive
(0x18002a8b0) contain **no** lambda — left untouched.

Our r7 built 5 of these lambdas inside `UiBridge::Push*`, so the wrappers were
named after `UiBridge` and the literal count read 21 vs the original's 28.

## My change (this round)

`src/Papyrus/Natives.cpp`, `Papyrus_SetSearchQuery` only: replaced the
`UiBridge::SetSearchQuery(a_query)` call with the original's inline
`SKSE::GetTaskInterface()->AddTask([query=std::move(a_query)](){...})` lambda,
which formats `window.slppSetSearchQuery(<QuoteJson(query)>);`, calls
`PrismaUI::InvokeOn(code)` (the bridge's single guarded dispatch, 0x180028190)
and then `Presentation::ApplyPresentation()` — matching 0x180024450's tail.

Predicted: one new literal matching
`_Func_impl_no_alloc<`void __cdecl `anonymous-namespace'::Papyrus_SetSearchQuery(...)'::`2'::<lambda_1>,void>`.

### Blocked on bridgeparams (HANDED OFF, not mine to edit)
- PublishSceneState / PublishCompatible call `UiBridge::PushState(json)` /
  `PushCompatible(json)` today. The original inlines the NO-ARG
  ``anonymous-namespace'::PushState()`` / ``PushCompatible()`` which read the
  global JSON and capture nothing. bridgeparams (sub-53) owns these; I will
  switch my call sites to `UiBridge::PushState()` / `PushCompatible()` the
  moment they land. Asked 2026-09-21.

### Handed to catalogbody (sub-54)
CatalogBegin/Append/Finish/Publish must instantiate their own lambdas inside the
`Papyrus_Catalog*` bodies (evidence + captures in the table above), not via
`UiBridge::PushCatalog*`. I am not editing those bodies; sub-54 owns them.

## Verification plan
- `strings <dll> | grep -c _Func_impl_no_alloc` (original 28, r7 21) — joint with
  bridgeparams, not my sole criterion.
- `tools/match.py --new <artifact> --focus 0x18002bd20` — SetSearchQuery ratio and
  first divergence.
- Confirm the SetSearchQuery wrapper's RTTI name equals the original's.
## Verification done locally (no CI yet)

Compile check of the INTEGRATED shape (my `src/Papyrus/Natives.cpp` on top of
`grind/bridgeparams`' headers) with clang-cl against `/tmp/CommonLibSSE-NG` +
`/tmp/xwin-sysroot`: **EXIT=0, 0 errors** (81 pre-existing CLNG offsetof
warnings). The scratch worktree was removed afterwards.

Objective check of the acceptance criterion (`strings` the object):
`/tmp/tlcheck/Natives.obj` contains
`?AV?$_Func_impl_no_alloc@V<lambda_0>@?0??Papyrus_SetSearchQuery...` (clang-cl
numbers the first lambda `0`; MSVC emits `<lambda_1>`) — i.e. the wrapper is now
instantiated **inside `Papyrus_SetSearchQuery`**, exactly like the original's
`_Func_impl_no_alloc@V<lambda_1>@?1??Papyrus_SetSearchQuery@?A0xbb2e73b6@...`.
Before this commit, that wrapper lived inside `UiBridge::SetSearchQuery`.

Caveat (open, not mine): the mangled anon-namespace hash our build emits is not
`?A0xbb2e73b6`, and bridgeparams' `UiBridge::PushState/PushCompatible` name the
wrapper `?1??PushState@UiBridge@@` instead of the original's
`?1??PushState@?A0xbb2e73b6@`. Count is right; byte-match of those two wrapper
names needs the TU to be `src\main.cpp` / global-anon PushState. Flagged to
bridgeparams + main.
