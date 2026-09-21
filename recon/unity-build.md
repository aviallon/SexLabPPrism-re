# Unity build — token evidence and required layout (grind/unitybuild)

Round 9, agent `unitybuild`. Question: is the original a single translation
unit, and can our separate TUs ever reproduce its `?A0x`-token RTTI names?

**Answer: the token grouping proves a single unity TU.** Every plugin-level
`?A0x` literal in the original carries one of exactly TWO tokens, and the
functions under each token are coherent file groups. Our separate-TU build has
FOUR tokens. Details and the one honest caveat (whether the token value, not the
layout, is reproducible) below.

Sources: `grep -aoE '\?A0x[0-9a-f]+' artifacts/SexLabPPrism.dll | sort | uniq -c`
(and the same on `artifacts/rebuild/parity-r8.dll`); `strings -a -n 6`.

## 1. Original — token -> function table

Exactly two distinct tokens (raw-byte grep, so this is not a `strings` artifact):

```
26  ?A0xbb2e73b6
12  ?A0xa6da2f39
```

### `?A0xbb2e73b6` — the unity TU's GLOBAL anonymous namespace (26 occurrences)

All of the plugin's bridge/native/state code. Function families visible in the
mangled strings (`_Func_impl_no_alloc<lambda...>::` RTTI descriptors and their
`<lambda_1>@...` companions):

| function | mangled evidence |
|---|---|
| `InputSink` (class, top level) | `.?AVInputSink@?A0xbb2e73b6@@` |
| `MenuVisibilitySink` (class, top level) | `.?AVMenuVisibilitySink@?A0xbb2e73b6@@` |
| `DispatchAction` | `_Func_impl_no_alloc<lambda_1 ... DispatchAction@?A0xbb2e73b6@@YAXV?$basic_string...` |
| `Papyrus_CatalogAppend` | `...Papyrus_CatalogAppend@?A0xbb2e73b6@@...` |
| `Papyrus_CatalogBegin` | `...Papyrus_CatalogBegin@?A0xbb2e73b6@@...` |
| `Papyrus_CatalogFinish` | `...Papyrus_CatalogFinish@?A0xbb2e73b6@@...` |
| `Papyrus_CatalogPublish` | `...Papyrus_CatalogPublish@?A0xbb2e73b6@@...` |
| `Papyrus_SetSearchQuery` | `...Papyrus_SetSearchQuery@?A0xbb2e73b6@@...` |
| `PublishCatalogToUi` | `...PublishCatalogToUi@?A0xbb2e73b6@@YAX_K@Z...` |
| `PushCompatible` (no-arg) | `...PushCompatible@?A0xbb2e73b6@@YAXXZ...` |
| `PushState` (no-arg) | `...PushState@?A0xbb2e73b6@@YAXXZ...` |
| `QueuePresentation` (no-arg) | `...QueuePresentation@?A0xbb2e73b6@@YAXXZ...` |
| `RequestSearchInput` (+ 2 nested lambdas) | `...RequestSearchInput@?A0xbb2e73b6@@YAXPEBD@Z...`, `...?R1?1??RequestSearchInput@?A0xbb2e73b6@@...` |

These live in *different source files* in our tree (ActionDispatch.cpp,
UiBridge.cpp, PrismaUI.cpp, Presentation.cpp, Papyrus/Natives.cpp,
InputSink.cpp, MenuVisibilitySink.cpp). **One token across all of them can only
be produced by one translation unit** — MSVC mangles `?A0x<hash>` per TU, so
separate TUs would diverge (and do — §2).

### `?A0xa6da2f39@FocusRecovery` — a nested namespace (12 occurrences)

A second, *distinct* unnamed namespace, nested inside `namespace FocusRecovery`:

| function | mangled evidence |
|---|---|
| `CloseConsolePulse` | `_Func_impl_no_alloc<lambda_1 ... CloseConsolePulse@?A0xa6da2f39@FocusRecovery@@YAX_K@Z` |
| `SendMenuMessage` | `...SendMenuMessage@?A0xa6da2f39@FocusRecovery@@YAXV?$basic_string_view...` |
| `StartConsolePulse` | `...StartConsolePulse@?A0xa6da2f39@FocusRecovery@@YAX_K@Z` |
| `CheckUnfocus` (2 lambdas) | `_Func_impl_no_alloc<lambda_1>@?7??CheckUnfocus@...` and `<lambda_2>@?1??CheckUnfocus@...` |
| `VerifyCleanup` | `_Func_impl_no_alloc<lambda_1>@?BB@??VerifyCleanup@?A0xa6da2f39@FocusRecovery@@YAX_KH_N@Z` |

`Begin@FocusRecovery` carries **no** `?A0x` token (`...Begin@FocusRecovery@@YAXPEAVIVPrismaUI1...`):
it is an ordinary free function of the named `FocusRecovery` namespace, so the
nested anonymous namespace exists alongside named functions. This is exactly the
shape the `FocusRecovery` code already documents in `src/FocusRecovery.cpp`.

**Conclusion.** Coherent grouping: one global plugin namespace (single unity TU)
+ one FocusRecovery-internal namespace. The unity-build hypothesis is confirmed.

## 2. Our build — tokens in `artifacts/rebuild/parity-r8.dll`

Raw-byte grep, four tokens, one per separate TU:

| token | TU / scope | literal |
|---|---|---|
| `?A0xd1b633de` | ActionDispatch.cpp global anon ns | `RequestSearchInput@?A0xd1b633de@@` (2) |
| `?A0xf42b6800@FocusRecovery` | FocusRecovery.cpp nested anon ns | `CloseConsolePulse@?A0xf42b6800@FocusRecovery` (2) |
| `?A0x1d4b42bd@InputSink` | InputSink.cpp nested anon ns (`Sink`) | `.?AVSink@?A0x1d4b42bd@InputSink@@` (1) |
| `?A0xf6965dd5@MenuVisibilitySink` | MenuVisibilitySink.cpp nested anon ns (`Sink`) | `.?AVSink@?A0xf6965dd5@MenuVisibilitySink@@` (1) |

Our bridge/native/state code emits **no** `?A0x` literal at all where the
original does, because on `main` those functions still live in *named*
namespaces (`namespace ActionDispatch`, `namespace UiBridge`, …) or in nested
anonymous namespaces; their mangled names therefore differ from the original's
`function@?A0xbb2e73b6@@` form long before the token value matters.

`_Func_impl_no_alloc` literal counts:

| | total | `<lambda>` wrappers | non-lambda (StaticFunctionTag method ptrs) |
|---|---|---|---|
| original | 28 | 19 | 9 |
| parity-r8 | 21 | 12 | 9 |

So the 7-literal gap is entirely lambda wrappers. It has **two independent
causes**: (i) the token/namespace shape of each function, and (ii) the source
shape (which functions convert a lambda into a `std::function`). r8 is missing
wrappers for `DispatchAction`, `Papyrus_Catalog{Append,Begin,Finish,Publish}`,
`PublishCatalogToUi`, `SendMenuMessage`, `StartConsolePulse`,
`CheckUnfocus` (x2), `VerifyCleanup`; and it has 3 extra that the original does
not (`add_papyrus_sink`, `remove_papyrus_sink`, `create@Trampoline` — CLNG
internals the original's whole-program build eliminated).

## 3. Token derivation — what reproduces the *value*

- MSVC names an anonymous namespace `?A0x<8 hex>` unique to the TU. LLVM
  review D50877 states MSVC hashes the **absolute path to the main source
  file**, while clang deliberately hashes the path **as passed on the command
  line** (XXH3_64 truncated to 32 bits). Verified locally with clang-cl 21:
  `main.cpp` -> `?A0x74C3D617`, and low32(XXH3_64("main.cpp")) == `74C3D617`.
- With **two** unnamed namespaces in one TU, clang reuses the same hex and only
  appends `@Scope`; the original shows two *different* hexes
  (`bb2e73b6` vs `a6da2f39`), so MSVC's value additionally encodes the unnamed
  namespace's scope/ordinal. Reproducing `a6da2f39` therefore needs the
  FocusRecovery namespace to be the **second** unnamed namespace of the unity
  TU (the global one being first).
- The original has **no** PDB path / RSDS and no drive-letter strings, so the
  author's absolute build path is not recoverable from the binary. Whether the
  token value is reproducible at all is exactly what the CI build settles: if
  MSVC hashed the relative `src\main.cpp` (which both builds use, visible in
  `__FILE__` literals), the unity TU will emit `bb2e73b6`; if it hashed the
  absolute path, it cannot, and a path-control trick would be required.

## 4. Required layout (implemented in `src/main.cpp` + `xmake.lua`)

1. `src/main.cpp` textually `#include`s every plugin implementation `.cpp`;
   xmake `remove_files()` excludes them from direct compilation. Done.
2. `main.cpp`'s own `namespace {}` (SetupLog) stays first, so the global
   unnamed namespace is declared before any nested one. Done.
3. `FocusRecovery.cpp` is included first among the farm so its nested anonymous
   namespace is the second unnamed namespace of the TU. Done.
4. **Still required from the other owners (round-9 source reshaping):** the
   nested anonymous namespaces in `ActionDispatch.cpp`, `UiBridge.cpp`,
   `PrismaUI.cpp`, `InputSink.cpp` and `MenuVisibilitySink.cpp` must go (their
   functions become global-anonymous or named-namespace members as the original
   has them). While those exist, the unity TU contains >2 unnamed namespaces and
   FocusRecovery cannot be the second one — the `a6da2f39` token stays wrong
   even if the path hash is right. This cannot be fixed from `main.cpp`.

## 5. Local compile evidence

clang-cl 21, all 11 impl files included into `src/main.cpp`, full CLNG/PCH
environment: **compiles clean, 0 errors** (50 warnings). The object now shows:

- `RequestSearchInput@?A0xFE46213C@@` (global token = main.cpp's TU) — the
  old `?A0xd1b633de` is gone;
- `CloseConsolePulse@?A0xFE46213C@FocusRecovery` — the nested token;
- plus residual nested tokens `@PrismaUI`, `@InputSink`, `@MenuVisibilitySink`
  from condition (4) above.

clang's hex differs from MSVC's by construction (different hash + our path), so
this validates the *layout*, not the value.

## 6. CI prediction (one build, unity layout on round-9 `main`)

- `?A0xbb2e73b6` present in the DLL: **conditional**. Layout makes it possible;
  it lands only if (a) the round-9 reshaping removes the extra nested
  namespaces, and (b) MSVC's hash input matches the original (relative path).
  If the hash is absolute-path-based, it will NOT appear — that is a defect of
  the original's build, not of this patch, and would need an explicit path
  control experiment.
- `?A0xa6da2f39@FocusRecovery`: same condition, plus FocusRecovery being the
  second unnamed namespace.
- `_Func_impl_no_alloc` total: token fix alone moves existing wrappers onto the
  unity token but does not add wrappers; count stays where the source shapes put
  it (r8 21) unless the missing lambdas (§2) are implemented.
- `match.py --focus`: functions whose bytes differ only in the `?A0x` token
  should improve from MISSING/RATIO towards MATCH; no-regression baseline is
  the corrected inventory (round-9 fix: contiguous .pdata fragments merged):
  **r8 535/911 real BYTE (58.7%), instruction-weighted 44.7%, MISSING 95**.

## 7. Addendum after rebasing on `main` @ `40227ab`

- `bridgeparams` landed: `InputSink`/`MenuVisibilitySink` are now top-level, so
  their nested `@InputSink`/`@MenuVisibilitySink` tokens are gone from the
  unity object. Remaining nested tokens in the unity object: `@FocusRecovery`
  (wanted) and `@PrismaUI` (owned by `grind/anonns`). ActionDispatch/UiBridge
  nested helpers emit no RTTI string, but still count as extra unnamed
  namespaces and must also be flattened.
- `src/Papyrus/CatalogNatives.cpp` (split out by `catalogbody`) cannot join the
  farm yet: its global anonymous namespace redefines `kClassName` and
  `QuoteJson`, which `Natives.cpp` also defines -> compile error. It stays a
  separate TU (keeping its own `?A0x` token) until those helpers are shared
  via a header.