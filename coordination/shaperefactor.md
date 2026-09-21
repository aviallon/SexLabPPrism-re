# shaperefactor agent (grind/shaperefactor) — native body shape

Branch `grind/shaperefactor`, owns `src/Papyrus/*`, `src/SceneState.*`,
`src/Catalog.*`. Commit `be0a59a`.

## 2026-09-21 — all 13 native bodies moved to the global anonymous namespace

### What changed
- `src/Papyrus/Natives.cpp` now holds, in the TU's **GLOBAL anonymous namespace**,
  the state + helpers + 13 free functions named `Papyrus_Log`,
  `Papyrus_BeginSceneSession`, `Papyrus_IsFreeCameraActive`,
  `Papyrus_PublishSceneState`, `Papyrus_PublishCompatible`,
  `Papyrus_CatalogBegin`, `Papyrus_CatalogAppend`, `Papyrus_CatalogPackage`,
  `Papyrus_CatalogFinish`, `Papyrus_IsCatalogReady`, `Papyrus_GetCatalogCount`,
  `Papyrus_CatalogPublish`, `Papyrus_SetSearchQuery` — exactly the original
  FUNCSIG names (verified with `strings artifacts/SexLabPPrism.dll`).
- Exact original signatures (by-value `std::string` / `std::vector<std::string>`).
  `Papyrus_CatalogAppend` is **three vectors** (decoded from the original's
  `_Func_impl_no_alloc` mangling `YAXPEAUStaticFunctionTag@RE@@V?$vector@...@std@@11@Z`;
  the round-7 note claiming `string,string` was wrong).
- Every log site pinned `#line N "src\\main.cpp"` (filename now set, not just line).
- Cross-TU accessors the other agents call were kept, defined in the same TU:
  `SceneState::{IsSceneActive,IsUiMode,SetUiMode,IsInterfaceHidden,
  ToggleInterfaceHidden,IsModalSearchOpen,SetModalSearchOpen,CurrentStateJson}`,
  `Catalog::RetryPublish`, `Catalog::Fnv1aHash::operator()`.
- `src/SceneState.cpp` / `src/Catalog.cpp` reduced to comment-only TUs (bodies moved).

### Countable signal (strings, per binary)
| binary | `_Func_impl_no_alloc` | anon-ns `Papyrus_` FUNCSIG |
|---|---|---|
| original | 28 | 9 |
| parity-r7 | 21 | **0** |
| this branch (predicted) | 21 (unchanged) | **9** |

The 9 anon-ns FUNCSIG literals are the objective, countable win: they come from
the 9 log-bearing native bodies now naming `anonymous-namespace'::Papyrus_X`.

### Before (measured, `tools/match.py --new artifacts/rebuild/parity-r7.dll --focus`)
| addr | verdict | ratio / first_div |
|---|---|---|
| 0x18002a8e0 Papyrus_Log | RATIO | 0.9863 / 24 (folded level) |
| 0x180029630 BeginSceneSession | RATIO | 0.7843 / 6 |
| 0x18002a8b0 IsFreeCameraActive | BYTE-MATCH | 1.0 |
| 0x18002ae90 PublishSceneState | MISSING | — |
| 0x18002a9a0 PublishCompatible | MISSING | — |
| 0x18002a000 CatalogBegin | MISSING | — |
| 0x180029740 CatalogAppend | MISSING | — |
| 0x18002a410 CatalogPackage | MISSING | — |
| 0x18002a270 CatalogFinish | MISSING | — |
| 0x18002a8a0 IsCatalogReady | MISSING | — |
| 0x18002a830 GetCatalogCount | RATIO | 0.6667 / 2 |
| 0x18002a6e0 CatalogPublish | MISSING | — |
| 0x18002bd20 SetSearchQuery | RATIO | 0.5698 / 3 |

### Predicted after CI — PREDICTIONS ONLY (one build requested)
| addr | before -> predicted |
|---|---|
| 0x18002a8e0 | RATIO -> RATIO (0.986 → ~same; only source_loc/FUNCSIG moved) |
| 0x180029630 | RATIO -> RATIO (name anchor now fires; body direct) |
| 0x18002a8b0 | BYTE-MATCH -> BYTE-MATCH |
| 0x18002ae90/0x18002a9a0/0x18002a000/0x180029740/0x18002a410/0x18002a270/0x18002a8a0/0x18002a6e0 | MISSING -> RATIO (FUNCSIG name anchor now pairs them) |
| 0x18002a830 | RATIO -> RATIO (body direct; no wrapper call) |
| 0x18002bd20 | RATIO -> RATIO (0.57 → up; wrapper call removed) |
- `_Func_impl_no_alloc`: predicted **unchanged at 21** — the wrapper instantiations
  still live inside `UiBridge::Push*`, see "remains". No claim that the 19
  original-only wrapper names drop from this change.

### Remains unconverted (honest)
1. **TaskInterface lambdas** — the native TaskInterface/std::function dispatch is
   still `UiBridge::Push*`, so the `_Func_impl_no_alloc<lambda...>` wrappers are
   not instantiated with a `Papyrus_X` enclosing name. Inlining that dispatch into
   each native body is the next step and is what should move the 28/21 count.
2. **`std::string_view` params** in `UiBridge::Push*` (the frame-size lever
   0xf0-vs-0xc0 etc.) untouched — bridge-owned file, needs coordination.
3. **Inlined FNV-1a id hash / row formatter (FUN_1800273a0)** in
   `Papyrus_CatalogAppend` still missing.
4. **Folded log level** (`r8d=0x2` into generic `log_`) is the LTCG link shape.
5. `CatalogFinish`/`IsCatalogReady` original sizes looked anomalous (7/1 insns) at
   the addresses the old pins named — worth re-checking the address/line mapping.

### Compile check
Not run: `build/tools/local-build.sh` exceeds the 45-min cap (round-7 experience).
This is a compile-unverified refactor; one CI build requested.
