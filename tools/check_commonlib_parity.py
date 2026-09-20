#!/usr/bin/env python3
"""Assert that a CommonLibVR checkout is the PRE-format-5 era the original used.

The shipped SexLabPPrism.dll was built against a CommonLibSSE-NG whose
REL::Module classifies the runtime with a *switch on the minor version* --

    switch (a_version[1]) { case 4: VR; case 6: AE; default: SE; }

-- which is why 1.7.104 (minor 7) falls into `default` and is treated as SE:
the plugin then looks for the SE-style `version-1-7-104-0.bin`, which does not
exist (the Address Library ships `versionlib-1-7-104-0.bin`). Later revisions
replaced this with a `>= 6` test and added Address Library format 5
(`include/REL/IDDB.h` with `Format::SSEv5`).

A parity build is only meaningful if it uses the same era, so this is a hard
gate: it fails the job rather than silently comparing different code. Every
conclusion is printed with the file:line evidence behind it.
"""
import pathlib
import re
import subprocess
import sys

root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "lib/CommonLibSSE-NG")
if not root.is_dir():
    print(f"::error::no such CommonLibSSE-NG checkout: {root}", file=sys.stderr)
    sys.exit(1)

print(f"parity-era check on {root}")
try:
    rev = subprocess.run(["git", "-C", str(root), "rev-parse", "HEAD"],
                         capture_output=True, text=True, check=True).stdout.strip()
    print(f"  revision: {rev}")
except Exception as exc:  # noqa: BLE001 - any failure here is a hard failure
    print(f"::error::could not resolve the checkout revision: {exc}", file=sys.stderr)
    sys.exit(1)

sources = []
for rel in ("include/REL", "src/REL", "CommonLibSSE/include/REL", "CommonLibSSE/src/REL"):
    base = root / rel
    if base.is_dir():
        sources += [p for p in sorted(base.rglob("*")) if p.suffix in (".h", ".hpp", ".cpp")]
if not sources:
    print("::error::found no REL sources to inspect - unexpected checkout layout", file=sys.stderr)
    sys.exit(1)
print(f"  inspecting {len(sources)} REL source files")


def scan(pattern):
    rx = re.compile(pattern)
    hits = []
    for path in sources:
        try:
            for lineno, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
                if rx.search(line):
                    hits.append((path.relative_to(root), lineno, line.strip()))
        except OSError:
            continue
    return hits


failures = []

# 1. Address-library format enum: SSEv1 present, SSEv5 absent.
v1 = scan(r"\bSSEv1\b")
v5 = scan(r"\bSSEv5\b")
print(f"  format enum: SSEv1 hits={len(v1)} SSEv5 hits={len(v5)}")
if v5:
    failures.append(f"checkout already supports format 5 ({v5[0][0]}:{v5[0][1]}); too new for parity")
if not v1:
    failures.append("no SSEv1 format enumerator found; not the expected era")

# 2. The SE/AE file naming must still be present.
lib_name = scan(r"versionlib-\{\}")
se_name = scan(r"version-\{\}")
print(f"  library file templates: versionlib-{{}}={len(lib_name)} version-{{}}={len(se_name)}")
if not (lib_name and se_name):
    failures.append("the version-{}.bin / versionlib-{}.bin naming templates were not found")

# 3. The minor-version classification must be the switch/equality form, not >= 6.
case6 = scan(r"case\s+6\s*:")
equality = scan(r"[=!]=\s*6\b")
gte6 = scan(r">=\s*6\b")
print(f"  classification: case 6={len(case6)} equality-vs-6={len(equality)} >=6={len(gte6)}")
if case6:
    print(f"    evidence: {case6[0][0]}:{case6[0][1]}  {case6[0][2]}")
elif equality:
    print(f"    evidence: {equality[0][0]}:{equality[0][1]}  {equality[0][2]}")
if gte6 and not (case6 or equality):
    failures.append(f"checkout classifies with >= 6 ({gte6[0][0]}:{gte6[0][1]}); too new for parity")
if not (case6 or equality):
    failures.append("no minor-version classification fingerprint (case 6 / equality vs 6) found")

if failures:
    for f in failures:
        print(f"::error::{f}", file=sys.stderr)
    print("::error::refusing to build a parity binary against the wrong era", file=sys.stderr)
    sys.exit(1)

print("  fingerprints: pre-format-5 enum (SSEv1, no SSEv5); SE/AE file naming; minor-6 classification")
print("OK: checkout is the expected pre-format-5 era")