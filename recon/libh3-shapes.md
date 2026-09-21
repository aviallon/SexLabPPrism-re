# H3 shape/instantiation forensics (round 7, grind/libh3)

Target: hypothesis **H3** of `recon/library-forensics.md` — 122 real library
functions / 3,657 instructions whose rebuild counterpart exists at a token ratio
in [0.5, 0.98). Method: find the type/shape the original's code implies from RTTI,
literals, call targets and operands, rather than rewriting bodies.

Inputs (r5 identity parity): `recon/library-forensics-real-split.json`,
`recon/library-forensics-bestmatch.json`. Current build:
`artifacts/rebuild/parity-r6.dll`. Tools: `tools/match.py --focus`,
`tools/vtable-analog.py`, `tools/pe_rtti.py`, `tools/h3-shapes.py`.

## 0. r5 → r6 progress on the 122 H3 addresses

`recon/matching/per-function.json` (parity-r6) now reports for the 122:
**5 BYTE-MATCH** (incl. the three disp32-only twins `0x18000d194`, `0x180005cee`,
`0x1800058ad` fixed by the round-6 displacement canonicalisation),
**18 INSN-MATCH**, **99 MISSING**. The 117 not-yet-byte functions carry
**3,050 instructions**.

## 1. The type inventory is essentially IDENTICAL — H3 is code shape, not types

Diffing the MSVC TypeDescriptor tables of the original and parity-r6
(`pe_rtti.py` on both, 195 vs 203 types) yields **exactly two** descriptors only
in the original:

| original TD | rebuild TD |
|---|---|
| `.?AVInputSink@?A0xbb2e73b6@@` | `.?AVSink@?A0x1d4b42bd@InputSink@@` |
| `.?AVMenuVisibilitySink@?A0xbb2e73b6@@` | (same nested-namespace form) |

So the `std::vector<RE::BSFixedString>` vs `std::vector<std::string>` class
(precedent 1) is **closed**: the container type inventory now matches. What is
left is (a) *where* our classes live (namespace/class nesting) and (b) the
*shape* of the code emitted for identical types — i.e. inlining/folding
boundaries and lambda-wrapper signatures.

Conversely, 10 TypeDescriptors exist **only in the rebuild**, of which 8 are
the already-known extra logging infrastructure the original does not have
(`msvc_sink<mutex>`, `_Ref_count_obj2<msvc_sink<mutex>>`, `LogEventHandler@Impl@SKSE`,
`BSTEventSink<LogEvent>`, `IStackCallbackFunctor`, `CallbackFunctor@Awaitable`,
`_Func_base<X,void*,unsigned __int64>`, `NiRefObject`); the other 2 are the
nested-namespace counterparts of the two above (`Sink@?A0x1d4b42bd@InputSink`,
`Sink@?A0xf6965dd5@MenuVisibilitySink`).

## 2. Evidence A — the `std::function` lambda-wrapper fingerprint

Every `std::function` constructed from a lambda instantiates
`std::_Func_impl_no_alloc<lambda_N...>`, whose name is emitted as a
`__FUNCSIG__` literal by spdlog's `source_location` macro at the *call site*.
Diffing those literals between the original and parity-r6 isolates exactly which
functions our source shapes differently.

**19 wrapper names present in the original, absent from parity-r6** (all in the
original's single anonymous namespace `?A0xbb2e73b6` or in `FocusRecovery`):

| original wrapper (signature shown by the mangled name) | owner to fix |
|---|---|
| `DispatchAction(std::string)` | implplugin2 |
| `Papyrus_CatalogBegin(StaticFunctionTag*, int)` | matchratio2 |
| `Papyrus_CatalogAppend(StaticFunctionTag*, vector<string>, string, string)` | matchratio2 |
| `Papyrus_CatalogFinish(StaticFunctionTag*)` | matchratio2 |
| `Papyrus_CatalogPublish(StaticFunctionTag*)` | matchratio2 |
| `Papyrus_SetSearchQuery(StaticFunctionTag*, std::string)` | matchratio2 |
| `PublishCatalogToUi(unsigned long long)` | implplugin2 |
| `PushState(void)` | implplugin2 |
| `PushCompatible(void)` | implplugin2 |
| `QueuePresentation(void)` | implplugin2 |
| `RequestSearchInput(const char*)` | implplugin2 |
| `RequestSearchInput(const char*)::<lambda_1>::operator()` | implplugin2 |
| `FocusRecovery::Begin(...)` | implplugin2 |
| `FocusRecovery::CheckUnfocus(unsigned long long,int)` (`<lambda_1>`, `<lambda_2>`) | implplugin2 |
| `FocusRecovery::VerifyCleanup(unsigned long long,int,bool)` | implplugin2 |
| `FocusRecovery::CloseConsolePulse(unsigned long long)` | implplugin2 |
| `FocusRecovery::StartConsolePulse(unsigned long long)` | implplugin2 |
| `FocusRecovery::SendMenuMessage(string_view, UI_MESSAGE_TYPE)` | implplugin2 |

**10 wrapper names present only in parity-r6** — these are the *wrong artifacts*
our current source produces:

```
.?AV?$_Func_impl_no_alloc@V<lambda_1>@?1??PushState@UiBridge@@YAXV?$basic_string_view...   (string_view!)
.?AV?$_Func_impl_no_alloc@V<lambda_1>@?1??PushCompatible@UiBridge@@YAXV?$basic_string_view...
.?AV?$_Func_impl_no_alloc@V<lambda_1>@?1??SetSearchQuery@UiBridge@@YAXV?$basic_string_view...
.?AV?$_Func_impl_no_alloc@V<lambda_1>@?1??PushCatalog@UiBridge@@YAXV?$basic_string_view...H
.?AV?$_Func_impl_no_alloc@V<lambda_1>@?1??PushCatalogChunk@UiBridge@@YAXV?$basic_string_view...HH
.?AV?$_Func_impl_no_alloc@V<lambda_1>@?1??QueuePresentation@Presentation@@YAXXZ...
.?AV?$_Func_impl_no_alloc@V<lambda_1>@?L@??RequestSearchInput@?A0xd1b633de@@YAXPEBD@Z...   (different anon-namespace hash!)
   (+ add_papyrus_sink/remove_papyrus_sink/Trampoline::create from SKSE's logger)
```

Interpretation, supported by the mangled names alone:

1. **Namespace shape.** The original keeps `QueuePresentation`, `PushState`,
   `PushCompatible`, `PublishCatalogToUi`, the `Papyrus_*` natives,
   `DispatchAction` and `RequestSearchInput` as **free functions in one
   anonymous namespace** (`?A0xbb2e73b6`). Our build splits them into named
   namespaces `UiBridge` and `Presentation` plus a different anonymous namespace
   (`?A0xd1b633de`). Any function whose `__FUNCSIG__`/lambda type is tied to that
   namespace can never match while the layout differs.
2. **Signature shape.** Original `PushState`/`PushCompatible`/`QueuePresentation`
   take **no arguments** (`YAXXZ`); ours take `std::string_view`. Original
   `Papyrus_SetSearchQuery` takes `std::string`; `UiBridge::SetSearchQuery` takes
   `std::string_view`.
3. **Class nesting.** `InputSink`/`MenuVisibilitySink` are top-level classes in
   the original's anonymous namespace; ours are nested (`InputSink::(anonymous
   namespace)::Sink`), which is the direct cause of the two type-descriptor-only
   diffs in §1.

This is the same class of fix as the round-6 precedents and it is **not**
fixable from build configuration.

## 3. Evidence B — import-thunk (`E8` → `FF 25`) vs IAT-indirect call

Three H3 functions differ from their rebuild twin **only** at one imported-call
site:

| addr | insn | rebuild | ratio | divergence |
|---|---|---|---|---|
| `0x180010070` | 21 | `0x180003e90` | 0.9524 | idx 11: `call 0x180051792` vs `call QWORD PTR [rip+0x63a32]` |
| `0x1800102f0` | 23 | `0x180003e30` | 0.9565 | idx 11: `call 0x180051792` vs `call QWORD PTR [rip+0x63a92]` |
| `0x180017e30` | 45 | `0x180007950` | 0.9778 | idx 44 (last): `call 0x180050363` vs `call QWORD PTR [rip+0x5fd98]` |

`0x180051792` in the original is a linker import thunk: `ff 25 80 4e 00 00  jmp
*0x4e80(%rip)` → IAT `0x180056618`, an **ordinal import from VCRUNTIME140.dll**
(the `memcpy`/`memmove`/`memset` thunk block at `0x180051780`). The original
therefore emits a direct `call <thunk>` (`E8 rel32`, 5 bytes); our `/GL`
whole-program build resolves the import to an indirect IAT call (`FF 15`, 6
bytes) with no thunk. Same mechanism as the `_Mtx_lock` divergence reported by
`matchratio` on `Papyrus_Log`/`BeginSceneSession`. **Hypothesis (not proven):**
this is a consequence of LTCG scope, not of source — round 1 measured
`PRISM_LTO_SCOPE=off`/`clng` as a large *overall* regression, so no config change
is proposed.

## 4. Evidence C — retained template/iterator duplicates (folding boundary)

Five pairs of near-identical originals map to single rebuild functions:

| originals (insn) | shared rebuild analog | consumed by |
|---|---|---|
| `0x18003e4cd/63d/7ad/91d/ea8d` (49 each) | `0x180010137` ratio 0.5366 | `0x180009e87` |
| `0x18003e479/5e9/759/8c9/ea39` (26 each) | `0x1800179dd` ratio 0.5778 | `0x18001135d` |
| `0x1800466d0` + `0x180046910` (183 each, ICF pair) | `0x180056a60` ratio 0.5014 | `0x1800119b0` |

Disassembly of `0x18003e479` shows the classic std::format
`back_insert_iterator` append loop (`mov 0x10(%rbx)` size, `cmp 0x18(%rbx)`
capacity, `call *(%rax)` grow, `mov %bpl,(%rcx,%rax)` store), matching the
`_Fmt_iterator_buffer<back_insert_iterator<basic_string<char>>>` vtable names in
`recon/parity-names-v2/names.json`. The original retained 5 instantiations that
our /GL + `/OPT:ICF` build folded into one. **Not fixable from source shape:**
the count difference is produced by the optimiser/linker folding boundary.

## 5. Evidence D — the std::format instruction region

44 of the 122 H3 addresses lie in the original's format/iterator region
`0x18003a000..0x180047000`; the largest H3 by mass are all there:

| addr | insn | rebuild | ratio | evidence |
|---|---|---|---|---|
| `0x18003d710` | 303 | `0x18004d2c0` | 0.733 | calls `0x18003f5d0/0x18003fa40/0x18003fec0` (format internals); diverges at range/`std::string` construction (idx 173–190) |
| `0x18003af80` | 173 | `0x18004abc0` | 0.701 | format/string region |
| `0x1800464d0` | 166 | `0x180056860` | 0.772 | padding/alignment writer (`movb $0x20`, `sub $0x1,%ecx` on align) |
| `0x1800466d0`/`0x180046910` | 183+183 | `0x180056a60` | 0.501 | ICF pair (§4) |
| `0x18003eb30` | 134 | `0x180019480` | 0.575 | format region |

Because the type inventory matches (§1) and the compiler/STL/toolset are already
pinned to 14.44, these are **call-site-driven instantiation/folding-boundary**
differences: the exact `std::format` argument/format-string types passed by our
`src/` call sites select different inlined machinery. Fixing them means aligning
those call sites, not an xmake flag.

## 6. What is fixable where

### Request list — source changes (I do not edit `src/`)

For **matchratio2** (`src/Papyrus/*`, `src/Catalog.*`, `src/SceneState.*`):
* Move the `Papyrus_*` native entry points and their lambdas out of `UiBridge`
  and back into the shared anonymous namespace, and give the wrappers the
  original signatures: `Papyrus_SetSearchQuery(RE::StaticFunctionTag*,
  std::string)` (not `string_view`); `Papyrus_CatalogAppend(RE::StaticFunctionTag*,
  std::vector<std::string>, std::string, std::string)` — the remaining
  `CatalogBegin/Finish/Publish` keep the `StaticFunctionTag*` ABI already applied
  in `98a9be9`.
* Evidence: the 5 missing `_Func_impl_no_alloc` names in §2.

For **implplugin2** (`src/PrismaUI.*`, `src/Lifecycle.*`, `src/InputSink.*`,
`src/MenuVisibilitySink.*`, `src/ActionDispatch.*`, `src/FocusRecovery.*`,
`src/Presentation.*`, `src/UiBridge.*`, `src/main.cpp`):
* `InputSink`/`MenuVisibilitySink` must be top-level classes in the plugin
  anonymous namespace (`?AVInputSink@?A0x...@@`), not nested under
  `InputSink::(anonymous namespace)::Sink` — this is the two-type-descriptor diff
  in §1.
* Revert the `UiBridge`/`Presentation` split: `QueuePresentation`, `PushState`,
  `PushCompatible`, `PublishCatalogToUi`, `DispatchAction`, `RequestSearchInput`
  are free functions in the original's anonymous namespace, and
  `PushState`/`PushCompatible`/`QueuePresentation` take **no arguments** while
  ours take `std::string_view`. Evidence: the 10 new-only wrapper names in §2.
* `FocusRecovery` lambdas: `Begin`, `CheckUnfocus` (two lambdas),
  `VerifyCleanup`, `CloseConsolePulse`, `StartConsolePulse`, `SendMenuMessage`
  must be captured in `std::function` exactly as the original does; and
  `CreateViews::<lambda_2/4/5>::operator()(const char*)` funcsigs exist only in
  the original (`auto __cdecl`anonymous-namespace'::CreateViews::<lambda_N>
  ::operator ()(const char *) const`).

### Fixes in MY files
* `tools/vtable-analog.py` was pinned to `parity-r5.dll` + a stale
  `/tmp/rttinew` RTTI; parameterised so it can map against the current build
  (`--orig/--new/--rtti-new`). This is a measurement-tool fix, **not a build
  change**.
* `tools/h3-shapes.py`: codifies the classifications above from the two
  binaries (type-descriptor diff, `_Func_impl_no_alloc` name diff,
  import-thunk-vs-IAT first divergence) so the next round can re-measure.
* **No xmake/CLNG change is proposed**: the evidence (identical type inventory,
  pinned 14.44 toolset) does not support one, and the only config lever that
  touches §3 (`PRISM_LTO_SCOPE=off`) was already measured in round 1 as a large
  regression (BYTE 846 → 527).

## 7. Prediction for the source handoff (to be measured)

If `matchratio2` + `implplugin2` apply §6, and the orchestrator runs one parity
CI build, the **predicted** deltas on the H3 set are:
* `_Func_impl_no_alloc` name diffs: 19 original-only → 0; the 10 rebuild-only
  names disappear (verifiable directly from `__FUNCSIG__` literals, no score
  needed).
* Type descriptors: 2 original-only (`InputSink`, `MenuVisibilitySink`) → 0.
* No claim is made for the std::format mass (44 functions) or the folded
  duplicates (§4) until measured; those are *not* expected to move from the
  namespace/signature refactor alone.
