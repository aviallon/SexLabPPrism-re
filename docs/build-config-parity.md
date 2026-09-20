# Original-build configuration parity — SexLabPPrism.dll 0.6.1

Analysis only. Inputs (read-only):

- `artifacts/SexLabPPrism.dll` — original, linker 14.44, PE timestamp `0x6a5c8ed5`
  = **2026-07-19 08:46:13 UTC** (parsed from the COFF header; `artifacts/meta.ini`
  `installationfile = ... 2026-07-19T13-58Z ...` confirms a 2026-07-19 build).
- `artifacts/rebuild/SexLabPPrism-rebuild-v2.dll` — our xmake/CLNG-HEAD rebuild.
- `recon/` string/RTTI/functions dumps, `recon/parity-v2.md`, `tools/pe-compare.py`.
- CLNG clones: `/tmp/clng-verify` (alandtse/CommonLibVR, unshallowed `ng`),
  and the sibling pin in `/home/aviallon/Programing/Opensource/SexLabpp`
  (submodule HTTP URL `alandtse/CommonLibVR`, branch `ng`).

---

## 1. CommonLibSSE-NG revision

### 1.1 Fingerprints that are visible in the binary

| # | Fingerprint in `artifacts/SexLabPPrism.dll` | Where | Consequence |
|---|---|---|---|
| F1 | `REL::Module::load_version` computes `(_version[1] != 6) + 1` for the non-VR case | disassembly `0x18001d1c8`: `cmp ax,0x4 / je ->store 4`; `cmp ax,0x6 / setne al / inc al / mov [rbx+0x118],al`; `Runtime` enum is `AE=1,SE=2,VR=4` (`include/REL/Module.h:62-85`) | equality test against minor 6 — the **switch/ternary** form, **not** `>= 6`; 1.7.104 (minor 7) falls through to `SE` |
| F2 | `void __cdecl REL::IDDatabase::header_t::read(REL::IDDatabase::istream_t&, unsigned char)` | `recon/strings.txt:55`-adjacent `__FUNCSIG__` set (`recon/functions.txt`) | the pre-`binary_io` reader that takes an explicit **format byte** |
| F3 | `bool __cdecl REL::IDDatabase::load_file(class std::basic_string_view<wchar_t,...>, class REL::Version, unsigned char, bool)` | `recon/strings.txt` | `std::uint8_t` (non-`std::optional`) format param |
| F4 | `bool __cdecl REL::IDDatabase::load_csv(class std::basic_string_view<wchar_t,...>, class REL::Version, bool)` | `recon/strings.txt` | VR CSV path compiled in |
| F5 | `Data/SKSE/Plugins/version-{}.csv`, `versionlib-{}.bin`, `version-{}.bin` | `recon/strings.txt:340-342` | both SE and "AE" file names, **only formats 1/2** |
| F6 | `Unsupported address library format: {}` + `enum class Format` limited to `SSEv1/SSEv2` | `recon/strings.txt:211`; **no** `SSEv5`, `load_v5`, `header_v5` strings anywhere | **no format-5 support** |
| F7 | `Failed to find the id within the address library: {}`, `Failed to locate an appropriate address library with the path: {}`, `Required VR Address Library file {} does not exist`, `VR Address Library {} loaded only {} entries but expected {}. Please redownload.` | `recon/strings.txt:186-188, 204, 211-213` | exact wording of the pre-format-5 error set |

### 1.2 Method and candidates

`/tmp/clng-verify` was unshallowed (`git fetch --unshallow origin ng`) but its `ng`
history has been rewritten: the only root is `224773c42`, and there is an
import-root `942f44a08` (2026-07-31). Git history on `src/REL/Module.cpp` /
`include/REL/Module.h` shows the equality switch was **replaced by `>= 6`** in
`7b47c5a8f` ("feat(rel): support AE 1.7.99 address library format 5 (#299)",
2026-08-21), whose parent is `7a879cce3`. Everything in
`cd61f6620` (2024-03-24, "Merge upstream (#87)", which introduced `a_formatVersion`,
`load_csv`, `version-{}.csv`) … `7a879cce3` (2026-08-21) matches F1–F7; `7b47c5a8f`
and current HEAD (`adb3e2c4`, v8.4.0) do **not** (F1 becomes `>= 6`, F6 gains
`SSEv5`/`load_v5`, F3 becomes `std::optional<std::uint8_t>`). The day after the
original was linked, `ng` moved on to the format-5 branch, which is why our rebuild
uses `versionlib-{}` and `>= 6`.

**Exact commit: `1cc6b3999eb92b4d86fe08be28011ba679980081`**
(`chore(release): 4.38.0 [skip ci]`, 2026-07-17 07:43:48 UTC) — the `ng` HEAD at the
original's link timestamp (2026-07-19 08:46:13 UTC). `git log <ng-tip> --until='2026-07-19 08:46:00 +0000' -1`
selects it; at that commit `include/REL/Module.h` has `case 6`, `include/REL/ID.h`
has `version-{}.csv`/`versionlib-{}.bin`/`a_formatVersion`, and `SSEv5` is absent.

Independently corroborated by the sibling project: `SexLabpp/lib/CommonLibSSE-NG`
is pinned to **`e60c1238d558eb12f2d6f230605abbd568f7a76d`** ("chore(release): 4.39.2",
2026-07-22), 3 days after the link. Its `include/REL/Module.h` still has the
equality switch and its `include/REL/ID.h` still has `Format{SSEv1,SSEv2}`,
`void read(istream_t&, std::uint8_t)`, `load_csv` and `version-{}.csv` — an
**identical fingerprint set**. The two independent selections (build timestamp vs.
sibling submodule pin) fall in the same pre-format-5 window, so the exact SHA is
`1cc6b3999` on the evidence of the link date, and `e60c1238d` is the nearest
reproducible pin if a fixed SHA is required.

**Confidence.** High that the original is on the `cd61f6620`…`7a879cce3` lineage and
predates `7b47c5a8f`. Medium-high on `1cc6b3999` itself (build-time HEAD; the
fingerprint set is stable across that whole window, so earlier `ng` commits from
2024-03 onward cannot be excluded by these features alone). A second fingerprint
that changes inside the window would be needed to collapse the range fully; the
obvious candidates (spdlog RTTI class set, `Version` encoding) were not found to
vary inside the window during this pass.

**Consequence for the rebuild:** pin CLNG to `e60c1238d` (or `1cc6b3999`) instead of
`ng` HEAD. This alone will not reproduce F1 exactly (a `switch` and a
`>=`-with-`setne` are different sources), but it removes the `IDDB.h` / `Format::SSEv5`
divergence and restores the same error strings, RTTI and API shapes.

---

## 2. MSVC toolset: 14.44 = Visual Studio 2022 17.14

`artifacts/SexLabPPrism.dll` reports linker **14.44**; Rich header entries are in the
`35207/35228/35724` families (`recon/parity-v2.md`). `14.44` is the MSVC v143
toolset shipped by **Visual Studio 2022 17.14**.

Runner-image facts (actions/runner-images issue #14017, 2026-05-07, and the image
matrix it quotes):

- Before the June-2026 migration, `windows-latest`/`windows-2025` ran **Windows
  Server 2025 with Visual Studio 2022 17.14.\*** — i.e. exactly toolset/linker 14.44.
- From ~2026-06-08…06-15 the `windows-latest` and `windows-2025` labels moved to
  **Visual Studio 2026 (18.5\*)**, whose linker is 14.51 — this is what our rebuild
  was linked with (`artifacts/rebuild/...-v2.dll`, linker 14.51, Rich `35721/36256`).
- `windows-2022` still carries Visual Studio 2022 and remains the direct path to
  14.44. The VS 2026 image additionally installs
  `Microsoft.VisualStudio.Component.VC.14.44.17.14.x86.x64` "for compatibility", so
  14.44 is also selectable on `windows-latest`/`windows-2025` after the switch.

Selection command (either works; `-vcvars_ver` is the same switch):

```bat
:: VS 2022 image / VS 2026 image with the 14.44 component installed
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=14.44
:: or, from any developer prompt setup
call VsDevCmd.bat -arch=amd64 -host_arch=amd64 -vcvars_ver=14.44
```

For xmake (documentation only — `build/xmake.lua` is not changed here), the
toolset is selected with `xmake f --vs=2022` (or by adding
`--toolchain=cl` after a `vcvarsall -vcvars_ver=14.44` environment); the workflow
should either use `runs-on: windows-2022` or, on the VS 2026 image, run
`vcvarsall -vcvars_ver=14.44` before `xmake build`.

The original mixes Rich build ids (`35207`, `35228`, `35724`, plus `33145`/`30729`
from prebuilt import libs). Those are compiler *build numbers*, not toolset releases:
the mix means the link combined objects produced by slightly different
incremental-update builds of the 19.4x compiler and by shipped import libraries
(`Implib900` 30729). Any rebuild can match the toolset (14.44) but not the exact
per-object compiler build, so Rich-id equality is not an achievable goal.

**Residual risk if 14.44 is unavailable on the chosen image:** the nearest is
toolset 14.4x on VS 2022 17.13/17.15 or VS 2026 14.51; codegen (register
allocation, inlining, /OPT decisions) differs per function even between two builds
of the same toolset, so exact disassembly parity is impossible in the general case.
Pin `windows-2022` if 14.44 must be seen in the binary's linker field.

---

## 3. Flags inferred from the ORIGINAL binary

| Flag | Evidence in the original | Current `build/xmake.lua` | Change for parity |
|---|---|---|---|
| `/O2` (not `/Ox`) | CLNG's own CMake `Release` leaves `CMAKE_CXX_FLAGS_RELEASE` at the CMake/MSVC default `/O2 /Ob2` (no `/O[0-9x]` override in `CLNG/CMakeLists.txt` at the pinned revision); the original's error strings/logging call sites are emitted with standard `__FUNCSIG__` source-location expansion, consistent with plain Release | `set_optimize("fastest")` → xmake maps `fastest` to **`/Ox`** on MSVC | `set_optimize("faster")` → `/O2`; drop any `/Ob3` |
| `/Gy` (function-level linking) | `.pdata` holds 1495 per-function entries with 93.4% coverage and the binary has 59 byte-identical function pools (`recon/parity-v2.md` §0), i.e. the linker folded identical COMDATs — which is only possible with `/Gy` | not set explicitly | add `cl::/Gy` |
| `/GS` (buffer security check) | Load Config directory (data dir 10) present, size `0x140`, `SecurityCookie = 0x180095900` (points into `.data`), read from the original's PE | `/GS` is on by default (no `/GS-`) | none |
| Control Flow Guard **off** | `DllCharacteristics = 0x160` (NX_COMPAT + HIGH_ENTROPY_VA + DYNAMIC_BASE; **no** `0x4000` IMAGE_DLLCHARACTERISTICS_GUARD_CF); no `.gfids` section (sections are `.text`, `.rdata`, `.data`, `.pdata`, `.rsrc`, `.reloc`); no `__guard_check_icall_fptr` import | `cl::/guard:cf-`, no `.gfids` | none |
| `/d2FH4` (EH4) | imports `__CxxFrameHandler4` (VCRUNTIME140_1) | both original and rebuild import it | none |
| LTCG (`/GL` compile + `/LTCG` link) | Rich header contains `Utc1920_LTCG_CPP` (35207, ×3) and `Utc1920_LTCG_C` (35228, ×1), and many `Utc1920_POGO_I_CPP` entries; CLNG `CMakeLists.txt:126` `set(CMAKE_INTERPROCEDURAL_OPTIMIZATION "$<$<CONFIG:RELEASE>:ON>")` | no LTCG; CLNG objects are compiled normally | enable LTCG: `/GL` on compile, `/LTCG` on link (xmake `set_policy("build.optimization.lto", true)` or explicit flags) |
| `/permissive-` | CLNG `xmake.lua:248` `add_cxxflags("/EHsc", "/permissive-", "/Zc:preprocessor")` — the CLNG headers are built with it | `build/xmake.lua` does **not** pass `/permissive-` | add `cl::/permissive-` |
| `/Zc:enumTypes /Zc:preprocessor /Zc:templateScope` | CLNG `xmake.lua:259-261` | already present (same three) | none |
| C++ standard **23** | CLNG `CMakeLists.txt:64,159` `cxx_std_23`; original uses `std::format` (`recon/strings.txt` fmt paths) | `set_languages("cxx23")` | none |
| `/MD` (dynamic CRT) | imports `MSVCP140.dll`, `MSVCP140_ATOMIC_WAIT.dll`, `VCRUNTIME140.dll`, `VCRUNTIME140_1.dll` | `set_runtimes("MD")` | none |
| `/OPT:REF` + `/OPT:ICF` | `.reloc` present (3,228 B) and 59 byte-identical pools; both are the Release linker defaults once `/Gy`/`/GL` are on | xmake Release link defaults include `/OPT:REF`; ICF depends on `/Gy` | verify `/OPT:ICF` after `/Gy` is enabled (`link::/OPT:ICF`) |

### The six most impactful changes

1. **Toolset/linker 14.44** — `runs-on: windows-2022`, or
   `vcvarsall x64 -vcvars_ver=14.44` on the VS 2026 image (§2).
2. **Pin the CLNG revision** to `e60c1238d` / `1cc6b3999` instead of `ng` HEAD (§1).
3. **LTCG** — `/GL` + `/LTCG` (Rich `Utc1920_LTCG_CPP`, CLNG CMake IPO=ON).
4. **`/O2` not `/Ox`** — `set_optimize("faster")`, and `/Ob2`, not `/Ob3`.
5. **`/permissive-`** — CLNG xmake passes it; the plugin build currently does not.
6. **`/Gy`** — required to let `/OPT:ICF` match the original's 59 folded pools.

`/GS` on, CFG off, EH4, `/MD`, C++23 and the three `/Zc:` switches already match;
they are not changes.

---

## 4. Resource: `src/Version.rc`

Parsed from the original's `.rsrc` (`VS_FIXEDFILEINFO` at file offset `0x9f288`,
signature `0xFEEF04BD`, structure version `0x00010000`; `StringFileInfo/040904b0`,
`Translation 0x0409,0x04B0`). Reproduce exactly:

```rc
#include <windows.h>

VS_VERSION_INFO VERSIONINFO
 FILEVERSION     0,6,1,0
 PRODUCTVERSION  0,6,1,0
 FILEFLAGSMASK   0x17L
 FILEFLAGS       0x0L
 FILEOS          0x4L
 FILETYPE        0x1L
 FILESUBTYPE     0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "FileDescription",  "PrismaUI scene controller for SexLab P+"
            VALUE "FileVersion",      "0.6.1.0"
            VALUE "InternalName",     "SexLabPPrism"
            VALUE "LegalCopyright",   "SickBaddie | Unknown License"
            VALUE "ProductName",      "SexLabPPrism"
            VALUE "ProductVersion",   "0.6.1.0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x0409, 0x04B0
    END
END
```

Notes on fidelity:

- The original has **no** `OriginalFilename`, `CompanyName`, `Comments`,
  `LegalTrademarks`, `PrivateBuild` or `SpecialBuild` values — do not add them.
- `FILEFLAGSMASK 0x17L` / `FILEOS 0x4L` are deliberately non-default (the standard
  `rc.exe` mask is `0x3F`, the standard OS is `VOS_NT_WINDOWS32 = 0x40004`); they
  were read back from the file and must be hard-coded to match.
- `Translation` matches the `040904b0` block; `VarFileInfo` is required for the
  block to be found by tools.
- The current rebuild has **no `.rsrc` at all** (`recon/parity-v2.md` §Sections),
  because `build/xmake.lua` sets no version rule.

xmake on Windows: add the file to the target, and xmake invokes the toolset's
`rc.exe` for `.rc` inputs:

```lua
add_files("src/Version.rc")
```

Optionally `set_version("0.6.1")` for the filename only; the resource itself must
stay the hand-written block above, because xmake's generated version resource uses
different `FILEFLAGSMASK`/`FILEOS` defaults and would not be byte-identical.

---

## 5. Residual noise after a perfect config

Even with toolset 14.44 and the exact CLNG pin, per-function parity will not hold:

- **CLNG revision width** — F1–F7 are stable over 2024-03…2026-08, so an unknown
  earlier `ng` commit inside the window compiles `REL::Module::load_version`,
  `IDDatabase::*` and every CLNG header inline with the same sources; the exact SHA
  cannot be proven from this binary. *Removable* only by getting the author's actual
  submodule pin (sexlabpp's is `e60c1238d`).
- **Compiler build number** — Rich mixes `35207/35228/35724` (and `33145/30729` from
  import libs). Different incremental-update builds of the same 14.44 toolset give
  different instruction selection. **Not removable** (the author's own machine
  produced the mix).
- **LTCG inlining/scheduling decisions** — even with `/GL`, the interprocedural
  inliner is sensitive to object order and to which CLNG objects are already `.obj`.
  *Partially removable* by matching the build order, never exactly.
- **ICF tie-breaking** — `/OPT:ICF` keeps the first of several byte-identical
  COMDATs; the survivor depends on input order and on LTCG. *Removable in principle*
  (`/OPT:NOICF`) at the cost of diverging from the original, which clearly folded.
- **Hot/cold splitting and block ordering** — MSVC `/O2` reorders basic blocks and
  the linker places functions by `.text$` contribution order. Layout differences
  shift every subsequent RVA and the `.pdata`/`.reloc` tables wholesale.
  **Not removable** without the author's exact object stream.
- **PGO** — the Rich header's `Utc1920_POGO_I_CPP` tool names indicate the LTCG/PGO
  compiler images; if the original was also linked with profile data, the optimizer
  decisions are unrecoverable. *Not removable* (no `.pgd`/profile shipped).
- **Runtime layout bases** — original `.text` starts at RVA `0x1000` and
  `.rdata`/`.data`/`.pdata` at `0x56000/0x95000/0x9d000`; any source or flag change
  moves the entry point (`0x50e28` → `0x562a0` in v2). **Not removable**; parity
  must be judged per function after RVA normalisation (`tools/parity.py`).

What *is* removable: the CLNG revision (pin), the toolset (14.44), `/O2` vs `/Ox`,
`/permissive-`, `/Gy`, LTCG, and the `.rsrc` (§4). What is not: compiler build
numbers, ICF/pgo tie-breaks, block ordering and the resulting RVA shifts.

---

## 6. LTCG scope and the retained `msvc_sink` (2026-09-21)

The config in §3 assumed one LTCG setting for the whole build. Two measurements refine
that, both taken on the CLNG `1cc6b3999` pin.

### 6.1 The original does not run CLNG's `log::init()`; we do

CLNG `1cc6b3999`:
- `src/SKSE/API.cpp:97` — `if (a_log) { log::init(); log::info("{} v{}", ...); }`,
  inside `#ifdef ENABLE_SKYRIM_AE`, from `SKSE::Init(const LoadInterface*,
  const bool a_log = true)` (`include/SKSE/API.h:15`).
- `src/SKSE/Logger.cpp:129-140` — `log::init()` creates a logger named `"global"` with
  `basic_file_sink_mt` + `msvc_sink_mt` and pattern `[%T.%e] [%=5t] [%L] %v`.

Raw byte search in `artifacts/SexLabPPrism.dll`:

| symbol | original | our `-lto` build |
|---|---|---|
| `msvc_sink` | absent | present (`0x0b5266`) |
| `LogEventHandler` (CLNG `Impl`) | absent | present |
| `add_papyrus_sink` | absent | present |
| `[%T.%e]` (CLNG pattern) | absent | present |
| `[%Y-%m-%d %H:%M:%S.%e]` (author pattern) | present | absent |
| `basic_file_sink` | present | present |
| `wincolor` (`stdout_color_sink`) | present | present |

So the author's `SetupLog` (`0x180027d20`, `recon/BINARY-RECON.md` §4.3 step 1) uses
`basic_file_sink` + `stdout_color_sink` and the pattern `[%Y-%m-%d %H:%M:%S.%e] [%l] %v`,
and `SKSEPlugin_Load` (`0x18002e250`) calls `SKSE::Init` at `0x18002e265` without
materialising `a_log`. Since CLNG's `log::init()` and everything only it references
(`msvc_sink`, `LogEventHandler`, `add_papyrus_sink`) are absent from the original, LTCG
proved `log::init()` unreachable there — either `a_log=false` or CLNG built without
`ENABLE_SKYRIM_AE`. We call `SKSE::Init(a_skse)` (`src/main.cpp:50`) and keep all of it.
This is a **source-level** divergence; the fix belongs in `src/main.cpp` (see
`coordination/ltcg.md`).

### 6.2 Measured effect of the LTCG we already have

`tools/match.py` on `artifacts/rebuild/SexLabPPrism-parity-lto2.dll`, scored with
`tools/real-functions-score.py` (the `.pdata` denominator): library tier
**704/1,149 BYTE-MATCH**, exactly the same as `parity-lto.dll` and as the pre-LTCG
baseline. Target-scoped LTCG is therefore byte-neutral on the real library tier; the raw
matcher's larger library count (1,169/1,750) is inflated by gap-scan pseudo-functions.

### 6.3 `PRISM_LTO_SCOPE` — the CLNG-side LTCG experiment

`xmake.lua` now selects LTCG scope with `PRISM_LTO_SCOPE` (`target` default, `clng`,
`project`, `off`; `PRISM_NO_LTO=1` aliases `off`), and `.github/workflows/build.yml`
exposes it as the `lto_scope` `workflow_dispatch` input.

- `target` — `/GL` on our TUs only; CLNG compiled without `/GL` (current, measured).
- `clng` — project-scope policy set *before* `includes()` so CLNG is compiled with
  `/GL`, then turned back off on our target. This is the reading that fits both
  observations: the Rich header's `Utc1920_LTCG_CPP` and CLNG's
  `CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON` (§3) say CLNG itself was LTCG-merged, while
  the original's `InputSink::ProcessEvent` still makes a real `call` to a standalone
  `FocusRecovery` function instead of inlining it the way our target-scoped `/GL` does
  (`sub-32` measurement).
- `project` — project-wide policy set *before* `includes()`: CLNG and this target both
  `/GL`; the strongest whole-program reading of the Rich header.
- `off` — no LTO.

The `LNK2001 __std_regex_transform_primary_char` that first blocked project-scope LTCG
came from the auto-downloaded **prebuilt** CLNG release library (a foreign toolset). The
parity job now drops CLNG's `.git` so no prebuilt can be fetched and CLNG compiles from
source with 14.44; if the symbol still fails to resolve from source-built `/GL` objects,
the remaining candidate is MSVC STL `std::regex` internals in `Logger.cpp`, which we also
remove once §6.1 is applied (the author's binary has no `std::regex` `LogEventHandler`),
so the two fixes are complementary.

### 6.4 Flag-sweep priority after the measurements

Target-scoped LTCG measured byte-neutral (§6.2), so the remaining build-mode flags are
low-yield compared with reconstructing the original's source shape. Ranked:

1. **`PRISM_LTO_SCOPE=clng`** (§6.3) — the only flag with a positive *mechanism*: it
   makes CLNG's spdlog/std/regex instantiations LTCG-merged as the original's Rich
   header implies, without letting `/GL` inline our plugin TUs against the original's
   per-function call graph.
2. **`PRISM_LTO_SCOPE=project`** — same, with our TUs `/GL` too; run it mainly to see
   whether the historical `LNK2001 __std_regex_transform_primary_char` reappears from
   source-built 14.44 objects. If it links, compare against `clng`.
3. **`/Gw`** — the only flag in the requested list not already fixed by `/O2` + the
   existing `/Gy`/`/Zc:inline`; MSVC `/O2` does not enable it and neither does CMake
   Release, so it is currently *off* on both sides and expected to be neutral. Test
   only if 1–2 leave a block of data-heavy functions unmatched.
4. **Not worth a build**: `/Ob3` (CLNG CMake Release is `/Ob2`; ours is `/Ob2`),
   `/Oi` and `/Ot` (already in `/O2`), `/Oy-` (no-op on x64), `/Zc:inline` and `/Gy-`
   (both already on / already matched), `/OPT:NOICF` (the original demonstrably folded
   59 identical pools, so NOICF can only move away from it; use it as a diagnostic to
   *count* ICF-induced mergers, never as a candidate config).
