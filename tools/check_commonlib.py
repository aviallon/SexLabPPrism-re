#!/usr/bin/env python3
"""Assert that a CommonLibSSE-NG checkout reads Address Library format 5.

SexLabPPrism 0.6.1 loads Data/SKSE/Plugins/versionlib-1-7-104-0.bin, which is
Address Library FORMAT 5. A CommonLibSSE-NG revision without include/REL/IDDB.h
(or without the SSEv5 enumerator / load_v5 reader) cannot read it and the plugin
dies at SKSE::Init with "Unsupported address library format: {}". This script
is called by .github/workflows/build.yml before xmake runs and must fail loudly.

Usage:
    python tools/check_commonlib.py [path-to-CommonLibSSE-NG]

Exit code 0 only when:
  * include/REL/IDDB.h exists, AND
  * `SSEv5` appears inside the `enum class Format` body, AND
  * a format-5 reader (`load_v5`) is declared.
"""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path


def fail(msg: str) -> None:
    print(f"::error::check_commonlib: {msg}", file=sys.stderr)
    sys.exit(1)


def git_revision(repo: Path) -> str:
    """Resolved revision, printing the full and short hash."""
    try:
        full = subprocess.run(
            ["git", "-C", str(repo), "rev-parse", "HEAD"],
            check=True,
            capture_output=True,
            text=True,
        ).stdout.strip()
        short = subprocess.run(
            ["git", "-C", str(repo), "rev-parse", "--short", "HEAD"],
            check=True,
            capture_output=True,
            text=True,
        ).stdout.strip()
        if full:
            return f"{full} ({short})"
    except (subprocess.CalledProcessError, FileNotFoundError):
        pass
    # Fall back to the submodule gitlink in the superproject, if present.
    try:
        rel = repo.resolve().relative_to(Path(__file__).resolve().parents[1])
        out = subprocess.run(
            ["git", "-C", str(Path(__file__).resolve().parents[1]), "ls-tree", "HEAD", str(rel)],
            check=True,
            capture_output=True,
            text=True,
        ).stdout.strip()
        if out:
            return out.split()[2] + " (gitlink; source tree not present)"
    except Exception:  # noqa: BLE001 - best-effort revision display only
        pass
    return "unknown"


def main(argv: list[str]) -> int:
    repo = Path(argv[1]) if len(argv) > 1 else Path(__file__).resolve().parents[1] / "lib" / "CommonLibSSE-NG"
    repo = repo.resolve()

    print(f"CommonLibSSE-NG checkout: {repo}")
    print(f"resolved revision:        {git_revision(repo)}")

    if not repo.is_dir():
        fail(f"{repo} is not a directory (did the workflow check out CommonLibVR?)")

    iddb = repo / "include" / "REL" / "IDDB.h"
    if not iddb.is_file():
        fail(f"missing {iddb} — this revision predates the format-5 address library reader")

    text = iddb.read_text(encoding="utf-8", errors="replace")

    # `SSEv5` must be an enumerator of `enum class Format`, not merely mentioned.
    enum_match = re.search(r"enum\s+class\s+Format\s*\{(?P<body>.*?)\}", text, re.DOTALL)
    if not enum_match:
        fail("include/REL/IDDB.h has no `enum class Format` — cannot read format-5 libraries")
    enum_body = enum_match.group("body")
    if "SSEv5" not in enum_body:
        fail("`enum class Format` has no `SSEv5` enumerator — unsupported address library format")

    if "load_v5" not in text:
        fail("include/REL/IDDB.h has no `load_v5` reader — format-5 support is incomplete")

    # Informational only, never fatal: older revisions word the comment differently.
    if "format-5" not in text and "format 5" not in text:
        print("note: no 'format-5' comment found in IDDB.h (continuing)")

    print("OK: include/REL/IDDB.h defines Format::SSEv5 and declares load_v5 (Address Library format 5)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))