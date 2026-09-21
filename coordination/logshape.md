# logshape agent (grind/logshape) — link-shape round

Owns (this run): `xmake.lua`, `.github/workflows/build.yml`, `docs/`, `tools/`,
`src/main.cpp` LOGGER SETUP only. Full write-up: `docs/logshape.md`.

## 2026-09-21 — BOTH link-shape differences are an LTCG (target /GL) regression

### Mechanism (evidence in docs/logshape.md §2-3)
1. **Log call** — our `/GL` build lets LTCG constant-propagate `lvl=level::info`
   into spdlog's header template `logger::log_<Args>` and emits a **specialised
   clone** (`cmp ebx,0x2`, no `r8d`). The original calls the generic body with
   `r8d=0x2`. Not spdlog config: original has zero fmt symbols and is
   `SPDLOG_USE_STD_FORMAT` like us; `SPDLOG_ACTIVE_LEVEL` would delete the call,
   not clone it; both sides instantiate the same header-only `log_` template and
   the same CLNG `SKSE::log::info` (Logger.h identical at `1cc6b3999` and HEAD).
2. **CRT lock** — original routes `_Mtx_lock` through a link-time `jmp [IAT]`
   thunk (`0x18004fb4b`); our `/GL` build calls `[IAT]` directly at 21 sites and
   the thunk at 16. Not `/MD` vs `/MT` (both import `MSVCP140.dll`/`VCRUNTIME140`)
   and not vcruntime-vs-api-ms (original imports `_Mtx_lock` from MSVCP140.dll,
   hint 1413).

### Signature counts (byte scan; docs/logshape.md §3 table)
| build | LTO | generic log_ | spec log_ | Mtx thunk/IAT |
|---|---|---|---|---|
| ORIGINAL | — | 21 | 0 | 37 / 0 |
| `rebuild/SexLabPPrism-parity.dll` (= ci-parity 09-20 17:15) | off | 19 | 0 | 30 / 0 |
| `parity-lto`/`lto2`, `parity-r3..r7`, `ci-parity2`, `ci-p3`, target | /GL | 9-10 | 16-18 | 16 / 21 |

The **only** no-LTO parity artifact reproduces the original's shape on both axes.

### Score (real-function denominator, tools/real-functions-score.py)
- no-LTO `SexLabPPrism-parity.dll`: **933/1495 BYTE** (plugin 165, library 768)
- LTCG `parity-lto`/`lto2`: 847; `parity-r7`: 848
Caveat: 933 and 847 straddle source commit `a7f6827`, so this is directional, not
a clean A/B. §6.2 of the old build-config doc used `rebuild-v2` (24.5%) as the
"pre-LTCG baseline" instead of `parity.dll` (62.4%), so target=846 was only ever
compared against other `/GL` scopes. The `recon/parity-runs/clng|project`
artifacts are the old **prebuilt** CLNG (their log bodies use the old
`log(loc,level,const T&)` overload), so the 527 number cannot rule out LTO scope.

### Implemented (my files)
- `.github/workflows/build.yml`: parity job defaults `PRISM_LTO_SCOPE=off`
  (was `''` → `target`); `lto_scope` input still sweeps target/clng/project;
  shipping job untouched.
- `xmake.lua`: comment only, `off` unchanged.
- `docs/logshape.md`: full mechanism + evidence.

### ONE CI build request (to orchestrator)
Run the **parity** job once with the new defaults (`PRISM_LTO_SCOPE=off`, pinned
CLNG `1cc6b3999`, 14.44). **Predictions, to be measured, not claimed:**
- `0x180029630 Papyrus_BeginSceneSession`: first divergence moves off insn #6
  (`call _Mtx_lock`) and off the log call — a no-LTO artifact already shows the
  lock call and the `r8d=0x2` log call matching, with only frame offsets and the
  `0x174` source line left. `RATIO 0.7843 -> ~0.88` (still RATIO until
  matchratio2's `#line`/file/namespace refactor lands).
- `0x18002a8e0 Papyrus_Log`: first divergence moves from the log call to the
  source_loc line immediate (`0x16d` vs `0x1f`); the call shape disappears.
  `RATIO 0.9863` (first_div=call) -> the remaining diff is matchratio2's
  `src\main.cpp` / anon-namespace source_loc, not mine.
- Aggregate real BYTE: `848 -> >=900` (no-LTO prior measured 933 at an earlier
  source state); this is the figure to confirm.

### Not fixable here
- The `/GL` clone itself cannot be disabled while keeping `/GL`; the call sites
  are byte-identical source (`SKSE::log::info`), so there is no source-shape fix.
- RVA/layout and the source_loc file/namespace remain other agents' work.
