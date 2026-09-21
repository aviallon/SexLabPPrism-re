# libh3 agent (grind/libh3) — H3 library shape/instantiation round

Owns `tools/`, `docs/`, `xmake.lua`, `.github/workflows/build.yml`. Did **not**
touch `src/`. Full write-up: `recon/libh3-shapes.md`; machine data:
`recon/h3-shapes-data.json`.

## 2026-09-21 — H3 root cause is source namespace/signature shape, not types or build config

### Headline
The H3 set is 122 real library functions / 3,657 instructions (r5). On parity-r6:
5 BYTE-MATCH (incl. the three round-6 disp32 twins), 18 INSN-MATCH, 99 MISSING —
117 functions / 3,050 instructions still open.

MSVC TypeDescriptor diff original ↔ parity-r6 is **2 original-only**
(`InputSink`, `MenuVisibilitySink`, both class-nesting) and **10 rebuild-only**
(8 are the known extra spdlog/SKSE sink infrastructure the original lacks —
`msvc_sink<mutex>`, `LogEventHandler@Impl@SKSE`, … — and 2 are the nested
counterparts of the two above). The container/type-mismatch class
(`vector<BSFixedString>` vs `vector<string>`) is **closed**. The remaining mass is
(a) code shape from inlining/folding boundaries and (b) 19 missing
`std::_Func_impl_no_alloc<lambda...>` wrapper names.

### Handoffs (I cannot edit src/)

**→ matchratio2** (`src/Papyrus/*`, `src/Catalog.*`, `src/SceneState.*`)
5 original `_Func_impl_no_alloc` lambda wrappers are absent in parity-r6; the
mangled names give the exact required signatures:
- `Papyrus_SetSearchQuery(RE::StaticFunctionTag*, std::string)` — ours is
  `UiBridge::SetSearchQuery(std::string_view)`
- `Papyrus_CatalogBegin(RE::StaticFunctionTag*, int)`
- `Papyrus_CatalogAppend(RE::StaticFunctionTag*, std::vector<std::string>, std::string, std::string)`
- `Papyrus_CatalogFinish(RE::StaticFunctionTag*)`
- `Papyrus_CatalogPublish(RE::StaticFunctionTag*)`
These must live as free functions in the plugin anonymous namespace (the
original's `?A0xbb2e73b6`), not in `UiBridge`.

**→ implplugin2** (`src/PrismaUI.*`, `src/Lifecycle.*`, `src/InputSink.*`,
`src/MenuVisibilitySink.*`, `src/ActionDispatch.*`, `src/FocusRecovery.*`,
`src/Presentation.*`, `src/UiBridge.*`, `src/main.cpp`)
- `InputSink` / `MenuVisibilitySink` are top-level classes in the original's
  anonymous namespace (`?AVInputSink@?A0xbb2e73b6@@`); ours are nested as
  `InputSink::(anonymous namespace)::Sink` (`?AVSink@?A0x1d4b42bd@InputSink@@`).
  This is the 2-type-descriptor diff.
- The known extra log infrastructure (`msvc_sink<mutex>`,
  `LogEventHandler@Impl@SKSE`, `add_papyrus_sink`) contributes 8 rebuild-only
  TypeDescriptors; removing it from `src/main.cpp`'s log setup is a separate
  parity item (round 1 already flagged it).
- Revert the named `UiBridge`/`Presentation` split for `QueuePresentation`,
  `PushState`, `PushCompatible`, `PublishCatalogToUi`, `DispatchAction`,
  `RequestSearchInput`: the original has them as free functions in the shared
  anonymous namespace, and `PushState`/`PushCompatible`/`QueuePresentation`
  take **no arguments** (`YAXXZ`) while ours take `std::string_view`.
- Restore the original's `std::function` lambda captures for
  `FocusRecovery::{Begin, CheckUnfocus, VerifyCleanup, CloseConsolePulse,
  StartConsolePulse, SendMenuMessage}` and the `CreateViews::<lambda_2/4/5>
  ::operator()(const char*)` handlers.

Evidence for all of the above: the exact missing/new wrapper name sets in
`recon/h3-shapes-data.json` and `recon/libh3-shapes.md` §2.

### Build-config verdict: no xmake change proposed
- The CI toolset is already pinned to MSVC 14.44 (`tools/check_linker.py`), the
  same toolset as the original, and the RTTI type inventory matches — so no
  `/std`, STL-version or define lever is supported.
- The only config lever that touches a real H3 sub-class (direct `call <import
  thunk>` vs `call QWORD PTR [IAT]`, 3 functions / 89 insns: `0x180010070`,
  `0x1800102f0`, `0x180017e30`) is LTCG scope; `PRISM_LTO_SCOPE=off/clng` was
  already measured in round 1 as a large regression (846 → 527 BYTE), so it is
  **not** proposed.
- Folded-template duplicate clusters (`0x18003e479/…`, `0x18003e4cd/…`, the
  `0x1800466d0`/`0x180046910` ICF pair) come from the optimiser folding boundary
  and are not source- or flag-reachable.

### Fixes in my files (implemented)
- `tools/vtable-analog.py` un-pinned from `parity-r5.dll` / stale `/tmp/rttinew`;
  now takes `--orig/--new/--rtti-orig/--rtti-new` (defaults to parity-r6) and
  recovers RTTI on demand.
- `tools/h3-shapes.py` added: emits the type-descriptor and
  `_Func_impl_no_alloc` differences as JSON for re-measurement.

### ONE CI build request (prediction, to be measured)
After matchratio2 + implplugin2 land the §Handoffs changes, the orchestrator
runs one parity build. **Predicted, to be measured** (not claimed):
- `_Func_impl_no_alloc` wrapper names: original-only 19 → 0; rebuild-only 10 → 0
  (countable straight from `__FUNCSIG__` literals, independent of the scorer).
- Type descriptors original-only: 2 → 0; rebuild-only 10 → 2 (the 8
  spdlog/SKSE sink types are the known extra infrastructure; removing them is a
  separate `src/main.cpp` log-setup change for implplugin2).
- No prediction is made for the 44 std::format-region functions or the folded
  duplicates; those are explicitly *not* expected to move from this refactor,
  and must be measured rather than assumed.
