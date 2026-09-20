# Dependency parity — what the original SexLabPPrism.dll was built against

Analysis of `artifacts/SexLabPPrism.dll` (linker 14.44, PE timestamp
`2026-07-19 08:46:13 UTC`) plus the two build manifests of the CLNG revision the
binary is on. Read alongside `build-config-parity.md` (which covers the CLNG
commit) and `recon/strings.txt` / `recon/rtti.txt` (which carry the raw evidence).

The purpose of this document is to make the library tier reproducible: the
~1,750 library/CLNG/spdlog functions can only byte-match if the exact same
dependency sources are compiled, so each dependency is listed with the evidence
that pins it and a mechanical sweep to settle anything that cannot be pinned from
the binary alone.

---

## 0. The two candidate manifests

CLNG v4.38.0 = `1cc6b3999eb92b4d86be08be28011ba679980081` ships **two** dependency
manifests, and they disagree. Which one the original used is itself a finding.

### A. xmake (CLNG `xmake.lua`)
```
add_requires("directxmath 2024.02", "directxtk 24.2.0")           -- line 65
add_requires("spdlog v1.16.0", { configs = { header_only = false,
              wchar = true, std_format = true } })                -- line 66
add_requires("rapidcsv v8.92")                                    -- line 85
```

### B. vcpkg manifest (CLNG `vcpkg.json`, baseline `ee12231b20c95013c6638d845d04c91559a1d1ff`)
```
directxmath  >= 2025-04-03
directxtk    >= 2025-10-27
spdlog       >= 1.16.0
rapidcsv     >= 8.90
```
Resolving the baseline's own version database at `ee12231b...` gives the exact
versions vcpkg would install:

| package | baseline-exact version |
|---|---|
| directxmath | **2025-04-03** |
| directxtk | **2025-10-27** |
| rapidcsv | **8.90** |
| spdlog | **1.16.0** |

The two manifests differ on DirectXMath, DirectXTK and rapidcsv, and they build
spdlog **completely differently**: xmake forces `std_format=true`
(`SPDLOG_USE_STD_FORMAT`, no fmt at all), while the vcpkg port at this baseline
declares `"dependencies": ["fmt"]` and builds with `-DSPDLOG_FMT_EXTERNAL=ON`
(`ports/spdlog/portfile.cmake:29`).

---

## 1. Which manifest the original used — the decisive binary evidence

**The original contains no fmt at all.**

| probe on `artifacts/SexLabPPrism.dll` | result |
|---|---|
| `fmt@@` (inline-namespace mangling in fmt symbols) | **0 occurrences** |
| `format_error@fmt` / `basic_format_string` | absent |
| `.?AVformat_error@std@@` (MSVC `std::format` exception) | **present** |
| MSVC STL `<format>` diagnostics (`Invalid fill (too long).`, `Format specifier requires numeric argument.`, `Invalid presentation type for bool`, …) | present (`recon/strings.txt`, fixed-message block) |
| spdlog's own `invalid fill character '{'` (`pattern_formatter-inl.h`) | present |

A vcpkg-built spdlog 1.16.0 always links/propagates external `fmt`, and fmt's
`throw_format_error` and `fmt::vN::` mangling would then be in the image. They are
not. The only configuration of CLNG v4.38.0 that produces this binary is the
**xmake** one (`std_format = true`). This also matches the sibling upstream build
of the same author: `/home/aviallon/Programing/Opensource/SexLabpp/xmake-requires.lock`
resolves the CLNG packages to the xmake versions.

> Confidence: high for spdlog (direct symbol/string evidence). DirectXMath /
> DirectXTK / rapidcsv are header-only or dead-stripped in the binary, so their
> versions are pinned by *the same manifest that the spdlog evidence selected*
> plus the author's lock file — not independently from the binary. That residual
> uncertainty is exactly what the sweeps in §3 remove.

---

## 2. Per-dependency verdict

### spdlog — **v1.16.0**, `header_only=false, wchar=true, std_format=true`
- `lib/CommonLibSSE-NG/xmake.lua:66` (CLNG v4.38.0) pins `spdlog v1.16.0` with those configs.
- `SexLabpp/xmake-requires.lock`: `["spdlog v1.16.0#09c8c173"]` (the `#09c8c173`
  suffix is a config hash distinct from the other packages' `#f56260b5`), against
  xmake-repo commit `33a3d2592b35c2e02b01b0824a9f017a787ddec1`.
- Binary: no fmt (above) ⇒ `SPDLOG_USE_STD_FORMAT`; spdlog RTTI formatter set in
  `recon/rtti.txt` is **identical** to our v1.16.0 parity build (incl.
  `wincolor_stdout_sink`, `basic_file_sink`, `level_formatter`, `a/b/c/A/B/C/Y`
  date formatters). v1.16.0 is correct.

### DirectXMath — **2024.02** (rank 1) vs 2025-04-03 (rank 2)
- Rank 1: `CLNG xmake.lua:65` + `SexLabpp/xmake-requires.lock` (`directxmath 2024.02`),
  same manifest that produced the spdlog evidence.
- Rank 2: vcpkg baseline exact `2025-04-03`, only reachable on the path the fmt
  evidence excludes.
- Not independently visible: DirectXMath is header-only and inlined; no RTTI, no
  version string. Settled by sweep `§3.2`.

### DirectXTK — **24.2.0** (rank 1) vs 2025-10-27 (rank 2)
- Rank 1: `CLNG xmake.lua:65` + lock `24.2.0`.
- Rank 2: vcpkg baseline exact `2025-10-27`.
- No DirectXTK RTTI/strings in the original (dead-stripped), so not independently
  visible. Settled by sweep `§3.3`.

### rapidcsv — **v8.92** (rank 1) vs 8.90 (rank 2)
- Rank 1: `CLNG xmake.lua:85` + lock `rapidcsv v8.92`.
- Rank 2: vcpkg baseline exact `8.90`.
- Binary marker: `.?AVno_converter@rapidcsv@@` in `recon/rtti.txt`
  (`TD=0x00096a98`). It proves rapidcsv is compiled in but is the same class in
  both candidates. Settled by sweep `§3.4`, which disassembles the caller
  `REL::IDDatabase::load_csv` (the only rapidcsv consumer in the original).

### Build-scope note (not a version problem)
Our parity `.dll` contains `.?AV?$msvc_sink@Vmutex@std@@@sinks@spdlog@@`; the
original does **not**, even though both compile `SKSE/Logger.cpp` (which always
constructs `msvc_sink_mt`). This is LTCG dead-code elimination of the never-called
`SKSE::log::init()` in the original (CLNG built with IPO), not a spdlog version
difference. See the LTCG scoping in `build-config-parity.md`; do not chase it
through spdlog versions.

### xmake-repo recipe commit
`SexLabpp/xmake-requires.lock` records the recipes' provenance as
`33a3d2592b35c2e02b01b0824a9f017a787ddec1`. Package *sources* are pinned by
version, but the recipe (patches, CMake flags) can drift with the xmake-repo
checkout; the workflow input `xmake_repo_ref` pins it.

---

## 3. Sweep plan (mechanical)

`build/xmake.lua` now maps `PRISM_SPDLOG_VERSION`, `PRISM_DIRECTXMATH_VERSION`,
`PRISM_DIRECTXTK_VERSION`, `PRISM_RAPIDCSV_VERSION` onto `add_requireconfs()`
overrides applied **before** CLNG's `add_requires` runs. Empty ⇒ CLNG's own pin,
so the normal parity gate is unchanged. The `parity` job exposes all four
(plus `parity_commonlib_ref` and `xmake_repo_ref`) as `workflow_dispatch` inputs.

Trigger a sweep (Windows runner; GitHub CLI shown):

```bash
# 3.1 spdlog (baseline sanity: v1.16.0 must match rank-1 config)
gh workflow run build.yml \
  -f spdlog_version=v1.16.0 \
  -f xmake_repo_ref=33a3d2592b35c2e02b01b0824a9f017a787ddec1

# 3.2 DirectXMath: rank 1 vs rank 2
gh workflow run build.yml -f directxmath_version=2024.02
gh workflow run build.yml -f directxmath_version=2025-04-03

# 3.3 DirectXTK: rank 1 vs rank 2
gh workflow run build.yml -f directxtk_version=24.2.0
gh workflow run build.yml -f directxtk_version=2025-10-27

# 3.4 rapidcsv: rank 1 vs rank 2
gh workflow run build.yml -f rapidcsv_version=v8.92
gh workflow run build.yml -f rapidcsv_version=v8.90

# 3.5 combined rank-1 replay (all four, plus the locked recipe repo)
gh workflow run build.yml \
  -f spdlog_version=v1.16.0 -f directxmath_version=2024.02 \
  -f directxtk_version=24.2.0 -f rapidcsv_version=v8.92 \
  -f xmake_repo_ref=33a3d2592b35c2e02b01b0824a9f017a787ddec1
```

Each run produces the `SexLabPPrism-0.6.1-parity` artifact. Download it and score
against the original:

```bash
gh run download <run-id> -n SexLabPPrism-0.6.1-parity -D /tmp/sweep-<label>
python3 tools/parity_score.py \
    --orig artifacts/SexLabPPrism.dll \
    --parity /tmp/sweep-<label>/SexLabPPrism-parity.dll \
    --report recon/parity-score-<label>.md \
    --outdir recon/parity-score-<label>
python3 tools/match.py \
    --orig artifacts/SexLabPPrism.dll \
    --new  /tmp/sweep-<label>/SexLabPPrism-parity.dll --report
```

Decision rule, read **per-function**, not on the headline number:
- DirectXMath: compare functions that inline XM* code (the library tier's
  `RE::NiPoint3` / `RE::hkVector4` / `<DirectXMath.h>` helpers). The candidate
  with the higher BYTE-/INSN-MATCH count wins.
- DirectXTK: same, on any `DirectX::*` inlined call site; if the original has no
  DirectXTK call site reachable, the two binaries should be byte-identical in that
  region and the sweep is vacuous — say so rather than claiming a win.
- rapidcsv: compare `REL::IDDatabase::load_csv` (the rapidcsv-using function;
  `id2offset`/`read`/`istream_t` are already 1.0 on the pinned revision and are
  controls). The v8.90/v8.92 delta, if any, will show as a RATIO<1 in `load_csv`
  only.
- spdlog: `v1.16.0` is expected to be a no-op vs the current build; use §3.1 as
  the control that proves the override plumbing works (the `[prism] dependency
  override` line must appear in the xmake log and the artifact hash must change
  only if the pin differs).

### 3.6 CLNG commit sweep (still needed)
The CLNG commit is an **era match, not provably exact** (see
`build-config-parity.md` §1.2: everything in `cd61f6620`…`7a879cce3` matches the
seven binary fingerprints, and `1cc6b3999` is the build-timestamp HEAD). Run the
same scoring against candidate refs:

```bash
for ref in cd61f6620 e60c1238d 1cc6b3999 7a879cce3; do
  gh workflow run build.yml -f parity_commonlib_ref=$ref \
    -f xmake_repo_ref=33a3d2592b35c2e02b01b0824a9f017a787ddec1
done
```

The winning `ref` is the one whose library tier and the six named CLNG functions
(`REL::Module::load_version` 222/222, `Module::init` 127/127,
`IDDatabase::id2offset` 111/111, `header::read` 385/385, `istream_t` 385/385)
hold or improve; `1cc6b3999` currently ties them all at 0.99+.

---

## 4. What is not yet determined

1. DirectXMath/DirectXTK/rapidcsv are **not independently provable from the
   binary** (header-only/inlined or dead-stripped). They are pinned transitively
   by the same xmake manifest that the spdlog symbol evidence selects and
   corroborated by the author's own `xmake-requires.lock`. §3.2–§3.4 settle them.
2. The exact xmake-repo recipe commit only matters if a recipe changed between
   checkouts; `33a3d259…` is the lock-recorded one and is the default to sweep.
3. CLNG's exact commit remains an era match; a sweep over `cd61f6620…7a879cce3`
   with `tools/match.py` is required before calling it exact.