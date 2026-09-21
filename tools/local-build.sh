#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# tools/local-build.sh - build SexLabPPrism.dll locally on NixOS with clang-cl
# against the Microsoft CRT/Windows SDK obtained via xwin.  No Wine, no MSVC,
# nothing installed into the system or user environment.
#
# Usage (from the repo root):
#
#   nix shell nixpkgs#lld nixpkgs#llvmPackages.llvm \
#              nixpkgs#llvmPackages.clang-unwrapped \
#     -c bash tools/local-build.sh
#
# Requires:
#   * /tmp/xwin-sysroot           (create with the command below if missing)
#   * lib/CommonLibSSE-NG         (the pinned parity checkout, or any ng HEAD)
#
# One-time sysroot (629 MB), if /tmp/xwin-sysroot is absent:
#
#   nix run nixpkgs#xwin -- --accept-license --cache-dir /tmp/xwin-cache \
#       splat --output /tmp/xwin-sysroot
#
# Output:
#   build-local/SexLabPrism.dll   (Windows x64 PE, MSVC ABI, /MD like CI)
#   build-local/obj/...           (objects; the script is a full rebuild)
#
# This is a clang-cl build: it does NOT reproduce the CI parity build's MSVC
# 14.44 compiler or its /GL LTCG codegen, so it is NOT a byte-match oracle.
# It is an iteration loop: a source change can be compiled, linked and diffed
# against artifacts/SexLabPPrism.dll in ~3 minutes instead of a 16-minute CI
# round-trip.  See docs/local-build-loop.md for what it can and cannot decide.
#
# Priority: the script re-executes itself under SCHED_BATCH, nice 19 and the
# idle I/O class so a full rebuild does not make the machine unusable. Set
# PRISM_KEEP_PRIORITY=1 to opt out (e.g. when timing the build); each wrapper
# is probed and skipped with a warning if the kernel refuses it.
# ---------------------------------------------------------------------------
set -uo pipefail

# ---------------------------------------------------------------------------
# Low-priority scheduling. A build saturating every core at default priority
# makes the machine unpleasant to use, so this script re-executes itself under
# SCHED_BATCH, nice 19 and the idle I/O class. Each wrapper is probed first and
# skipped (with a note) if the kernel refuses it, so the build still runs.
# PRISM_LOW_PRIO=1 marks the re-executed pass and prevents an infinite loop;
# PRISM_KEEP_PRIORITY=1 opts out entirely (e.g. when timing the build).
# ---------------------------------------------------------------------------
if [ -z "${PRISM_LOW_PRIO:-}" ] && [ -z "${PRISM_KEEP_PRIORITY:-}" ]; then
    export PRISM_LOW_PRIO=1
    wrap=()
    if command -v ionice >/dev/null 2>&1 && ionice -c 3 true 2>/dev/null; then
        wrap+=(ionice -c 3)
    else
        echo "local-build: ionice idle class unavailable; continuing without it" >&2
    fi
    if command -v nice >/dev/null 2>&1 && nice -n 19 true 2>/dev/null; then
        wrap+=(nice -n 19)
    fi
    if command -v chrt >/dev/null 2>&1 && chrt -b 0 true 2>/dev/null; then
        wrap+=(chrt -b 0)
    else
        echo "local-build: SCHED_BATCH unavailable; continuing without it" >&2
    fi
    if [ ${#wrap[@]} -gt 0 ]; then
        echo "local-build: re-executing under: ${wrap[*]}"
        exec "${wrap[@]}" bash "$0" "$@"
    fi
fi

export PRISM_LOW_PRIO=1

echo "local-build: scheduling: nice=$(nice) class=$(LC_ALL=C chrt -p $$ 2>/dev/null | sed -n 's/.*scheduling policy: //p') io=$(LC_ALL=C ionice -p $$ 2>/dev/null | sed 's/^.*: //')"

PROJ=${PROJ:-$(pwd)}
CLNG=${CLNG:-$PROJ/lib/CommonLibSSE-NG}
XWIN=${XWIN:-/tmp/xwin-sysroot}
WORK=${WORK:-$PROJ/build-local}
JOBS=${JOBS:-$(nproc)}

SPD=${SPD:-/tmp/spdlog-1.16.0}
DXM=${DXM:-/tmp/DirectXMath}
DXT=${DXT:-/tmp/DirectXTK}
RCSV=${RCSV:-/tmp/local-build-inc}

die() { echo "local-build: $*" >&2; exit 1; }

[ -d "$XWIN/crt/include" ] || die "missing $XWIN - see the xwin command in this script's header"
[ -d "$CLNG/include/REL" ] || die "missing CommonLibSSE-NG checkout at $CLNG"
command -v clang-cl >/dev/null || die "clang-cl not on PATH (nix shell nixpkgs#llvmPackages.clang-unwrapped)"
command -v lld-link >/dev/null || die "lld-link not on PATH (nix shell nixpkgs#lld)"

# --- dependencies (headers only; spdlog is compiled into a static lib) ------
if [ ! -f "$SPD/include/spdlog/spdlog.h" ]; then
  echo "local-build: fetching spdlog v1.16.0 ..."
  ( cd /tmp && curl -sL -o spdlog-1.16.0.tar.gz \
      https://github.com/gabime/spdlog/archive/refs/tags/v1.16.0.tar.gz && \
    tar xzf spdlog-1.16.0.tar.gz ) || die "spdlog fetch failed"
fi
[ -d "$DXM/Inc" ] || { echo "local-build: fetching DirectXMath ..."; git clone --depth 1 https://github.com/microsoft/DirectXMath "$DXM" >/dev/null 2>&1; }
[ -d "$DXT/Inc" ] || { echo "local-build: fetching DirectXTK ...";  git clone --depth 1 https://github.com/microsoft/DirectXTK  "$DXT" >/dev/null 2>&1; }
mkdir -p "$RCSV"
[ -f "$RCSV/rapidcsv.h" ] || { echo "local-build: fetching rapidcsv v8.92 ..."; \
  curl -sL -o "$RCSV/rapidcsv.h" https://raw.githubusercontent.com/d99kris/rapidcsv/v8.92/src/rapidcsv.h || die "rapidcsv fetch failed"; }

INC="-imsvc $XWIN/crt/include -imsvc $XWIN/sdk/include/ucrt \
     -imsvc $XWIN/sdk/include/shared -imsvc $XWIN/sdk/include/um \
     -I $CLNG/include -I $CLNG/src -I $CLNG/extern/openvr/headers \
     -I $DXM/Inc -I $DXT/Inc -I $SPD/include -I $RCSV"
CXXFLAGS="--driver-mode=cl --target=x86_64-pc-windows-msvc $INC \
  /TP -std:c++23preview -EHsc -O2 /MD /Gy /Zc:inline /Zc:preprocessor /Zc:enumTypes \
  /Zc:templateScope /permissive- /utf-8 /bigobj \
  -DSPDLOG_USE_STD_FORMAT -DSPDLOG_COMPILED_LIB \
  -DENABLE_SKYRIM_SE=1 -DENABLE_SKYRIM_AE=1 -DENABLE_SKYRIM_VR=1"

rm -rf "$WORK/obj"; mkdir -p "$WORK/obj"/{clng,proj,spd}

# --- spdlog v1.16.0 compiled static lib -------------------------------------
SPDF="-DSPDLOG_COMPILED_LIB -DSPDLOG_USE_STD_FORMAT -DSPDLOG_WCHAR_TO_UTF8_SUPPORT=1"
for f in "$SPD"/src/*.cpp; do
  clang $CXXFLAGS $SPDF -c "$f" -o "$WORK/obj/spd/$(basename "$f" .cpp).obj"
done || die "spdlog compile failed"

# --- CommonLibSSE-NG sources ------------------------------------------------
find "$CLNG/src" -name '*.cpp' > "$WORK/clng-list.txt"
export CXXFLAGS CLNG WORK
cat "$WORK/clng-list.txt" | xargs -P "$JOBS" -I{} bash -c '
  f="{}"; rel=${f#"$CLNG"/src/}; out="$WORK/obj/clng/${rel//\//_}.obj"
  clang $CXXFLAGS -FI "$CLNG/include/SKSE/Impl/PCH.h" -c "$f" -o "$out"
' || die "CommonLibSSE-NG compile failed"
echo "local-build: clng objects $(ls "$WORK"/obj/clng/*.obj 2>/dev/null | wc -l)/$(wc -l < "$WORK/clng-list.txt")"

# --- project sources --------------------------------------------------------
# Unity build: src/main.cpp #includes all other src/*.cpp (see the include
# farm there and recon/unity-build.md), so it is the ONLY project TU to
# compile.  Compiling the included files separately would define every symbol
# twice and, worse, recreate the per-file `?A0x` tokens.
for f in "$PROJ/src/main.cpp"; do
  rel=${f#"$PROJ"/src/}; out="$WORK/obj/proj/${rel//\//_}"
  clang $CXXFLAGS -I "$PROJ/src" -FI "$PROJ/src/PCH.h" -c "$f" \
    -o "${out%.cpp}.obj" || die "project compile failed: $f"
done

# --- archive + link (no /GL: plain /O2, so no LTCG) -------------------------
ls "$WORK"/obj/spd/*.obj  | sed 's/^/"/;s/$/"/' > "$WORK/spd.rsp"
ls "$WORK"/obj/clng/*.obj | sed 's/^/"/;s/$/"/' > "$WORK/clng.rsp"
llvm-lib /nologo /out:"$WORK/spdlog.lib" @"$WORK/spd.rsp"
llvm-lib /nologo /out:"$WORK/clng.lib"   @"$WORK/clng.rsp"

lld-link /nologo /dll /machine:x64 /out:"$WORK/SexLabPrism.dll" \
  /libpath:"$XWIN/crt/lib/x86_64" /libpath:"$XWIN/sdk/lib/ucrt/x86_64" \
  /libpath:"$XWIN/sdk/lib/um/x86_64" \
  "$WORK"/obj/proj/*.obj "$WORK/spdlog.lib" "$WORK/clng.lib" \
  advapi32.lib bcrypt.lib d3d11.lib d3dcompiler.lib dbghelp.lib dxgi.lib \
  ole32.lib shell32.lib user32.lib version.lib \
  /OPT:REF /OPT:ICF /debug > "$WORK/link.log" 2>&1 || {
    tail -25 "$WORK/link.log" >&2; die "link failed (full log: $WORK/link.log)"; }

echo "local-build: OK -> $WORK/SexLabPrism.dll ($(stat -c%s "$WORK/SexLabPrism.dll") bytes)"