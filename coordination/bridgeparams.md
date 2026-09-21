# bridgeparams agent (grind/bridgeparams) — round 8 bridge shape

Branch `grind/bridgeparams` @ `3dfe834` (forks `main` after the revive merge).
Owns `src/UiBridge.*`, `src/PrismaUI.*`, `src/Presentation.*`,
`src/ActionDispatch.*`, `src/Lifecycle.*`, `src/FocusRecovery.*`,
`src/InputSink.*`, `src/MenuVisibilitySink.*`, `src/main.cpp`.

## 2026-09-21 — no-arg PushState/PushCompatible, top-level sinks, std::string

### What changed (`3dfe834`)
1. `UiBridge::PushState()` / `PushCompatible()` are **NO-ARG** and the queued
   lambdas capture **nothing**; they re-read the published globals
   (`SceneState::CurrentStateJson()` / `CurrentCompatibleJson()`) inside the
   task. This is the original's `anonymous-namespace'::PushState(void)::lambda_1`
   / `::PushCompatible(void)::lambda_1` shape (recon/libh3-shapes.md §2).
2. `InputSink` / `MenuVisibilitySink` are **top-level classes** in each TU's
   global anonymous namespace (was `InputSink::anon::Sink`). RTTI becomes
   `?AVInputSink@...` / `?AVMenuVisibilitySink@...` and `ProcessEvent` bakes
   ``anonymous-namespace'::InputSink::ProcessEvent``.
   Cross-TU registration is via `PrismSinks::RegisterInput()` /
   `PrismSinks::RegisterMenuVisibility()` (called from Lifecycle).
3. `Presentation::QueuePresentation` body moved to the global anonymous
   namespace (named shim retained for the InputSink call site).
4. Catalog/InvokeJs parameters are by-value `std::string`, not `string_view`
   (the 16-byte frame lever); `UiBridge::SetSearchQuery` is **deleted** — the
   native must inline its lambda (tasklambda).

### Exact dependency on src/Papyrus/Natives.cpp (tasklambda) — CI needs BOTH
```
L309  UiBridge::PushState(json);        -> UiBridge::PushState();
L332  UiBridge::PushCompatible(json);   -> UiBridge::PushCompatible();
L481  UiBridge::SetSearchQuery(a_query);-> inline the lambda:
        const std::string code = std::format("window.slppSetSearchQuery({});", QuoteJson(a_query));
        if (auto* t = SKSE::GetTaskInterface())
            t->AddTask([code]() { PrismaUI::InvokeOn(code); Presentation::ApplyPresentation(); });
NEW   std::string SceneState::CurrentCompatibleJson()
        { std::lock_guard lock{ g_jsonMutex }; return g_compatibleJson; }
```
`UiBridge.cpp` forward-declares `SceneState::CurrentCompatibleJson()`, so the
pair will not link until that accessor exists. `PushCatalog`/`PushCatalogChunk`
/`InvokeJs` param change (string_view→std::string) needs no call-site change.

### High-value finding for main/ltcg (NOT fixable from src/)
The original is a **unity build**. Its global anonymous namespace is
`?A0xbb2e73b6` (all main.cpp-level lambdas), while a nested anon ns in the SAME
original binary is `?A0xa6da2f39@FocusRecovery`. The `?A0x` token is per
(TU source path, anon-index): our `ActionDispatch.cpp` → `?A0xd1b633de`, nested
InputSink → `?A0x1d4b42bd`. Therefore the 19 original-only
`_Func_impl_no_alloc` RTTI names cannot be reproduced from separate TUs; only
by making the original's unity TU (`src/main.cpp` including the implementation
files, anon-ns order matching). The `strings | grep -c _Func_impl_no_alloc`
28-vs-21 gap is downstream of that layout, not of the individual signatures.

### Prediction (one CI build, target/off scope, measured vs artifacts/rebuild)
- `PushState`/`PushCompatible` wrapper RTTI changes from
  `...PushState@UiBridge@@YAXV?$basic_string_view...` to `...PushState@UiBridge@@YAXXZ`.
- Big natives' 16-byte frame delta closes on the parameter-type change.
- I do **not** predict 21 → 28 wrappers without the unity TU.
