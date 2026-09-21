# Local Windows build loop (clang-cl + xwin) — `tools/local-build.sh`

The four grinder agents need a sub-minute-per-change feedback loop instead of a
16-minute CI round-trip. This document records the working recipe, its output,
and — the part that decides how the grind works — exactly how far its verdicts
can be trusted against the acceptance gate.

## TL;DR

* **A local build loop exists.** One command builds a real Windows x64
  `SexLabPrism.dll` (MSVC ABI, opens in the same harness as the CI artifact).
* It is a **clang-cl** build: **not** MSVC 14.44 and **not** LTCG. It therefore
  is **not the acceptance oracle**. A local `BYTE-MATCH` is a *strong positive
  screen* (97.7 % of local BYTE-MATCHes are also CI BYTE-MATCH); a local
  `MISSING` says **nothing** (91 % of real functions are MISSING locally even
  when CI has them byte-matched).
* Use it to (a) prove a source change compiles and links, and (b) get a fast
  positive signal. **The acceptance gate stays the MSVC 14.44 CI parity build.**

## The command

```console
$ nix shell nixpkgs#lld nixpkgs#llvmPackages.llvm \
            nixpkgs#llvmPackages.clang-unwrapped \
    -c bash tools/local-build.sh
```

* Requires `lib/CommonLibSSE-NG` (the pinned `1cc6b3999` checkout, or any `ng`
  HEAD) with its `extern/openvr` submodule populated.
* Requires the one-time xwin sysroot at `/tmp/xwin-sysroot` (629 MB); the script
  errors with the exact `nix run nixpkgs#xwin …` command if it is missing.
* Fetches header-only deps into `/tmp` on first run: spdlog v1.16.0 (compiled
  into a static lib with `std_format` + wchar, matching CLNG's xmake config),
  DirectXMath, DirectXTK, rapidcsv v8.92.
* Full rebuild, ~7 min cold on 32 cores (`JOBS=24` inside the script), outputs
  `build-local/SexLabPrism.dll` (~792 KB, `/MD`, `/O2`, `/OPT:REF /OPT:ICF`,
  `/debug`, **no `/GL`**).

Correctness check of the loop itself: the script ran to `EXIT=0` and produced
`build-local/SexLabPrism.dll` = 792,064 bytes; the earlier hand-rolled run
(identical flags minus `/MD`) produced a DLL with 3 SKSE exports and the same
.reloc layout as CI.

## Comparability vs the CI parity artifact (the measured answer)

Built and scored with the shared oracle (`tools/match.py`), all 2,149 original
functions and then restricted to the 1,495 real (unwind-entry) functions:

| build | ALL BYTE | ALL INSN | real BYTE | real INSN | real MISSING |
|---|---|---|---|---|---|
| CI parity (`parity-lto2`, MSVC 14.44 + LTO) | 1358 | 19 | **847 (56.7 %)** | 19 | 407 (27.2 %) |
| local clang-cl, static CRT (first attempt) | 186 | 16 | 75 (5.0 %) | 11 | 1342 (89.8 %) |
| **local clang-cl, `/MD` (the script)** | 171 | 16 | **73 (4.9 %)** | 11 | 1366 (91.4 %) |

So local and CI are **not** score-equivalent, and the gap is not a flag bug:
clang-cl emits different instructions from MSVC 14.44 for the same source, and
the missing LTCG changes inlining. The `/MD` build reproduces CI's
base-relocation count (1521 vs CI's 1514) but that does not move byte-match.

### Is local useful for the acceptance gate? No. Is it useful for iteration? Partly.

Cross-tabulating the two runs on all paired functions:

* `local BYTE-MATCH → CI BYTE-MATCH`: **167 / 171 = 97.7 % precision**.
* `CI BYTE-MATCH → local BYTE-MATCH`: 167 / 1358 = 12.3 % recall.
* Of CI's 1,358 BYTE-MATCH functions, **1,133 are `MISSING` locally** — the
  local DLL's different codegen and function boundaries make the matcher fail to
  pair them at all.

Consequence for the other three agents:

1. **A local `BYTE-MATCH` is worth reporting immediately** — it is almost
   certainly a real CI match, and it costs ~7 min + ~3 min of scoring instead of
   16 min of CI.
2. **A local `MISSING`/`RATIO` on a function must not be read as "not close".**
   For most of the program the local build cannot even pair the function. Do not
   use local verdicts to *reject* an approach.
3. For an *edited* function, the local build reliably answers only one
   question: does it still compile and link, and what shape does clang-cl give
   it. Whether that shape matches the original's instruction stream is decided by
   the MSVC 14.44 CI build. Use `tools/match.py --focus` on the CI artifact for
   the acceptance verdict.

## Why the acceptance gate stays in CI

The original was built by MSVC 14.44 with `/GL /LTCG`. The parity job pins that
toolset and patches the linker version (`tools/check_linker.py … 14.44`). clang-cl
is ABI-compatible (MSVC mangling, MSVC STL layout) but a different compiler; a
clang-cl DLL cannot byte-match a 14.44 LTCG binary, and the measurements above
quantify that (4.9 % vs 56.7 %). No local flag change bridges it; only MSVC 14.44
does, and 14.44 is not installable here (Nix-only, no MSVC). So:

* local build = fast compile/link + positive screen;
* CI `SexLabPPrism-parity-lto` = the oracle for the >98 % target.

## What is still unsolved

* No local MSVC 14.44 toolset (deliberate: Nix-only, no Wine/MSVC download).
* No local LTCG (`/GL` requires an MSVC link-time code generator); adding
  `lld-link` LTO would not reproduce MSVC's codegen anyway.
* The script is a full rebuild (~7 min). An incremental mode keyed on object
  mtimes would make single-file iteration seconds, but is not implemented.
* `src/Version.rc` is not compiled into the local DLL (needs `rc.exe`/`llvm-rc`
  resource plumbing); the VERSIONINFO resource does not affect function bytes, so
  it is omitted to keep the loop simple.
## Scheduling (added after the first rounds)

A full local rebuild saturates every core, which made the machine unpleasant to
use during the grind, so `tools/local-build.sh` re-executes itself under:

* `chrt -b 0` - SCHED_BATCH (throughput-friendly, no interactivity theft)
* `nice -n 19` - lowest regular priority
* `ionice -c 3` - idle I/O class, so the build never competes with foreground work

Each wrapper is probed before use (`ionice -c 3 true`, `chrt -b 0 true`) and
skipped with a warning if the kernel refuses it. `PRISM_KEEP_PRIORITY=1` opts
out entirely, which is what you want when measuring build wall-clock.

Verified on this machine: a child under the wrapper reports `ni=19`,
`cls=B` (SCHED_BATCH) and `io=idle`, and the script prints the effective
scheduling on start, e.g. `scheduling: nice=19 class=SCHED_BATCH io=idle`.
