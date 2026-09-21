# Log-call and CRT-lock link shapes — root cause is LTCG (/GL), not a missing spdlog/CRT flag

Agent: `grind/logshape` (2026-09-21). Inputs: `artifacts/SexLabPPrism.dll`,
`artifacts/rebuild/SexLabPPrism-parity.dll` (no-LTO CI parity, 2026-09-20 17:15),
`artifacts/rebuild/SexLabPPrism-parity-lto{,2}.dll` and `parity-r*.dll` (/GL),
`recon/parity-runs/{target,clng,project}/artifact.dll`, `recon/imports.txt`.

## 1. What the two divergences actually are

`tools/match.py --focus 0x180029630` (Papyrus_BeginSceneSession, 51 insn) has two
first-order differences:

1. **Log call.** Original: `mov r8d,0x2` then `call 0x180021110`, where
   `0x180021110` reads the level from `r8d`
   (`mov r15d,r8d` / `cmp r8d,ebx` / `setge`). Ours: no `r8d`; `call 0x18002b610`,
   which has the level constant-folded (`cmp ebx,0x2` / `setle`).
   The original calls the *generic* `spdlog::logger::log_<int>`; ours calls a
   **specialised clone** with `lvl = 2` baked in.
2. **`_Mtx_lock`.** Original: `call 0x18004fb4b`, and `0x18004fb4b` is a 6-byte
   linker thunk `jmp QWORD PTR [rip+0x691f] # __imp__Mtx_lock`. Ours:
   `call QWORD PTR [rip+...] # __imp__Mtx_lock` directly (no thunk).

Both are **call-site shapes**, not logic. Neither is in `src/SceneState.cpp`.

## 2. The spdlog build config is NOT the cause

Checked and excluded:

- **fmt**: the original imports no `fmt::` symbol anywhere (`recon/imports.txt`);
  it is built with `SPDLOG_USE_STD_FORMAT` (`std_format = true`). Our build uses
  the same CLNG/spdlog xmake config (`header_only=false, wchar=true,
  std_format=true`, `xmake.lua` `prism_pin("spdlog", ...)`).
- **`SPDLOG_ACTIVE_LEVEL`**: if that macro folded the level it would *delete* the
  log call, not emit a call to a clone with a constant level. It does not explain
  a clone.
- **`SPDLOG_COMPILED_LIB` / `SPDLOG_HEADER_ONLY`**: both sides are
  header-instantiated (`logger::log_` is a template in
  `spdlog/logger.h:78-80,183`); the original has 21 generic `log_` bodies, so it
  is not a single compiled-lib function either.
- **Source shape**: `SKSE::log::info` is CLNG's
  `SKSE_MAKE_SOURCE_LOGGER(info, info)` class template
  (`include/SKSE/Logger.h`, identical at the pinned `1cc6b3999` and at `ng` HEAD),
  which always calls `spdlog::log(loc, level::info, fmt, args...)`. Same source
  construct on both sides. Therefore the difference is codegen, not source.

## 3. The mechanism: `/GL` (LTCG on our TUs)

Counting the two codegen signatures across every local artifact (byte scan):

| artifact | LTO scope | generic `log_` | specialised `log_` | `_Mtx_lock` thunk/IAT |
|---|---|---|---|---|
| `artifacts/SexLabPPrism.dll` (ORIGINAL) | — | **21** | **0** | **37 / 0** |
| `rebuild/SexLabPPrism-parity.dll` (= `ci-parity`, 09-20 17:15) | **off** | **19** | **0** | **30 / 0** |
| `ci-parity2`, `ci-p3`, `parity-lto`, `parity-lto2` | target | 9 | 16 | 16 / 21 |
| `parity-r3..r7` | target | 9-10 | 16-18 | 16 / 21 |
| `recon/parity-runs/target/artifact.dll` | target | 9 | 16 | 16 / 21 |
| `recon/parity-runs/{clng,project}/artifact.dll` | CLNG /GL (old prebuilt) | 0 | 0 | 0 / 34 |

Signature: generic = `8b 59 40 44 3b` (`mov ebx,[rcx+0x40]; cmp r8d,ebx`),
specialised = `8b 59 40 83 fb` (`mov ebx,[rcx+0x40]; cmp ebx,imm`).

Every `/GL` build has the specialised clone and the mixed/IAT lock call; the only
no-LTO build has the original's exact shape on both axes. LTCG's interprocedural
constant propagation folds `lvl = level::info` into the template instantiation and
also chooses the direct-IAT import reference instead of the link-time thunk. The
`recon/parity-runs/clng/*` artifacts are not comparable: their log bodies use the
old `log(loc, level, const T&)` overload (no `r9`, frame `0x130`), i.e. they were
built against the foreign auto-downloaded CLNG prebuilt lib — which is why the
historical "clng/project = 527" number cannot be used to rule out LTO scope.

## 4. Score arithmetic (confounded, read the caveat)

On the `.pdata` real-function denominator (`tools/real-functions-score.py`):

| build | LTO | real BYTE-MATCH |
|---|---|---|
| `rebuild/SexLabPPrism-parity.dll` (ci-parity 09-20 17:15) | off | **933 / 1495** (plugin 165, library 768) |
| `ci-parity2` / `parity-lto` / `parity-lto2` | target | 847 / 1495 (plugin 143, library 704) |
| `parity-r7` (current main) | target | 848 / 1495 |

That is **+86 BYTE functions for no-LTO**, i.e. the round-1 "target = oracle"
conclusion (846) was only ever a comparison *among* `/GL` scopes; the no-LTO
parity build was never used as the oracle, and §6.2 of the old
`build-config-parity.md` used `rebuild-v2` (24.5%) as its "pre-LTCG baseline"
instead of `parity.dll` (62.4%). Caveat: `ci-parity` (17:15) and `ci-parity2`
(21:42) are 4.5 h apart and straddle `a7f6827` (source), so the 933→847 delta is
not a perfectly controlled LTO A/B. The call-shape evidence in §3 is exact and
independent of the score; the aggregate must be confirmed by one CI run.

## 5. Fix implemented

`.github/workflows/build.yml`: the `parity` job now defaults
`PRISM_LTO_SCOPE=off` (was `''` → xmake default `target`). The `lto_scope`
workflow input can still select `target`/`clng`/`project` for sweeps, and the
shipping `build` job is untouched (LTO stays on there for performance).
`xmake.lua` is unchanged: `off` is already a supported scope and simply routes
around the `build.optimization.lto` policy.

Not fixable: the `target` specialisation itself cannot be disabled while keeping
`/GL`, and the spdlog source (`SKSE::log::info` → `spdlog::log(loc, const level,
...)`) is identical to the original, so there is no source-shape change that
removes the clone without removing `/GL`.
