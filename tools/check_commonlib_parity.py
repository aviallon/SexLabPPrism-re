#!/usr/bin/env python3
"""Assert that a CommonLibVR checkout is the PRE-format-5 era the original used.

The shipped SexLabPPrism.dll was built against a CommonLibSSE-NG whose
REL::Module::load_version classifies the runtime with an *equality* test
against minor 6 --

    (minor != 6) + 1        -> AE=1, SE=2

-- which is why 1.7.104 (minor 7) is treated as SE and the plugin looks for the
SE-style `version-1-7-104-0.bin` that does not exist. Later revisions replaced
that with a `>= 6` test and added Address Library format 5 (IDDB.h with
Format::SSEv5). This script asserts the checkout still has the old behaviour,
so a parity build cannot silently drift onto a revision that changes the code
being compared.

Exit 0 = the checkout is the expected era. Exit 1 = it is not; the job must fail.
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

evidence = []

# Evidence 1: the IDDB format enum must have SSEv1 but not SSEv5.
iddb = root / "include/REL/IDDB.h"
if iddb.is_file():
    text = iddb.read_text(errors="replace")
    has_v1 = "SSEv1" in text
    has_v5 = "SSEv5" in text
    print(f"  IDDB.h: SSEv1={has_v1} SSEv5={has_v5}")
    if has_v5:
        print("::error::IDDB.h already has format 5 (SSEv5); too new for the parity build",
              file=sys.stderr)
        sys.exit(1)
    if not has_v1:
        print("::error::IDDB.h has no SSEv1 format enumerator; unexpected revision",
              file=sys.stderr)
        sys.exit(1)
    evidence.append("IDDB.h has Format::SSEv1 and no SSEv5")
else:
    print("  IDDB.h: absent (pre-IDDB revision)")

# Evidence 2: the runtime classification must be the equality test against 6.
module = root / "src/REL/Module.cpp"
if module.is_file():
    text = module.read_text(errors="replace")
    equals6 = re.search(r"!=?\s*6\b|==\s*6\b", text)
    ge6 = re.search(r">=\s*6\b", text)
    print(f"  Module.cpp: equality-vs-6={bool(equals6)} greater-or-equal-vs-6={bool(ge6)}")
    if ge6 and not equals6:
        print("::error::Module.cpp uses the newer >= 6 classification; too new for parity",
              file=sys.stderr)
        sys.exit(1)
    if equals6:
        evidence.append("REL/Module.cpp still classifies with an equality test against minor 6")
else:
    print("  Module.cpp: missing; relying on the IDDB evidence")

if not evidence:
    print("::error::no parity-era fingerprint found; refusing to build a parity binary",
          file=sys.stderr)
    sys.exit(1)

print("  parity-era fingerprints: " + "; ".join(evidence))
print("OK: checkout is the expected pre-format-5 era")