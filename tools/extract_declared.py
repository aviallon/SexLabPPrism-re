#!/usr/bin/env python3
"""Extract address -> our-function declarations from the reconstructed sources.

The round-10/round-12 agents wrote bodies whose header comments record WHICH
ORIGINAL ADDRESS the body was decompiled from (e.g. MissingRound2Group1.cpp:
"0x180054d70 ... ShutdownCaptionStore").  Those comments are the only binding we
have for a MISSING original that has no name, no RTTI, no distinctive literal and
no call-graph position: the matcher cannot bind it, but the reconstruction
explicitly claims it.

This tool parses, from disk, every such declaration.  It never asks an agent and
never invents an address.  Two declaration kinds are recognised:

  * RECONSTRUCTED    a function definition in src/missing/*.cpp preceded by a
                     comment that carries the original address.  Evidence is
                     <file>:<line> of the function definition plus the comment.
  * PRESENT-UNPAIRED a "PRESENT-BUT-UNPAIRED" list entry in a reconstruction
                     header/body comment that cites an existing src/ file:line
                     body which was NOT duplicated.

Output columns (recon/declared-mappings.csv):
  orig_addr, our_symbol_or_source_location, evidence, round, kind

Usage:
  python3 tools/extract_declared.py [--src src] [--out recon/declared-mappings.csv]
  python3 tools/extract_declared.py --self-test
"""
from __future__ import annotations

import argparse
import csv
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)

ADDR_RE = re.compile(r"0x(18[0-9a-fA-F]{6,})\b")
# A function definition head: not a call, not a declaration, and the body opens
# on the same or the next line.  Handles "type name(args)", "type ns::name(args)",
# "extern \"C\" type name(args)", template return types and trailing qualifiers.
DEF_RE = re.compile(
    r"^\s*(?:extern\s+\"C\"\s+)?(?:__declspec\(noinline\)\s+)?"
    r"(?:static\s+)?(?:inline\s+)?"
    r"(?P<ret>(?:[\w:]+(?:<[^;{}()]*>)?[\s*&]+)+)"
    r"(?P<name>(?:[\w:]+::)?~?\w+)\s*\((?P<args>[^;{}]*)\)"
    r"(?:\s*const)?(?:\s*(?:noexcept|override|final))*\s*(?:\{|$)"
)
SKIP_NAMES = {"if", "for", "while", "switch", "return", "sizeof", "alignof",
              "namespace", "struct", "class", "enum"}


def round_of(path: str) -> str:
    base = os.path.basename(path)
    if base.startswith("MissingRound2"):
        return "round2"
    if base.startswith("MissingGroup"):
        return "round1"
    return "pre-worklist"


def name_matches(blob, name):
    """The declared body's comment must name the function it declares."""
    n = name.lower()
    if n in blob:
        return True
    return any(len(part) >= 6 and part in blob
               for part in re.split(r"[^a-z0-9]+", n))


def brace_depth(lines, start, end):
    return sum(lines[i].count("{") - lines[i].count("}") for i in range(start, end))


def function_def(lines, i):
    """If a function definition head starts at lines[i], return the name.

    Handles multi-line parameter lists (the round-2 files wrap long
    signatures) and Allman/K&R braces.
    """
    line = lines[i]
    s = line.lstrip()
    if not s or s.startswith(("//", "#", "/*", "*")):
        return None
    m = re.match(
        r'^\s*(?:extern\s+"C"\s+)?(?:__declspec\(noinline\)\s+)?'
        r'(?:static\s+)?(?:inline\s+)?'
        r'(?:[\w:]+(?:<[^;{}()]*>)?[\s*&]+)+(\w+)\s*\(', line)
    if not m:
        return None
    name = m.group(1)
    if name in SKIP_NAMES or "=" in line[:m.start(1)]:
        return None
    # find the signature's closing paren (may be 1-3 lines later)
    j = i
    buf = line[m.end():]
    steps = 0
    while ")" not in buf and j + 1 < len(lines) and steps < 4:
        j += 1
        buf += " " + lines[j].strip()
        steps += 1
    if ")" not in buf:
        return None
    after = buf.split(")", 1)[1]
    if "{" in after:
        return name
    k = j + 1
    while k < len(lines) and lines[k].strip() == "":
        k += 1
    if k < len(lines) and lines[k].strip().startswith("{"):
        return name
    return None


def find_declarations(path: str):
    """Yield (addr, symbol, evidence, kind) for every reconstructed body.

    Walk contiguous comment blocks.  A declaration block is a block whose FIRST
    address line also carries ``insn`` or ``GUESS`` (the round-1/round-2 body
    header); later addresses inside the same prose (data offsets such as
    0x18009c1d8, call targets) are ignored.  The block's function is the first
    definition after it, skipping helper struct/class bodies in between
    (e.g. CaptionSlot before ShutdownCaptionStore).
    """
    lines = open(path, encoding="utf-8").read().splitlines()
    rel = os.path.relpath(path, REPO)
    out = []
    i = 0
    while i < len(lines):
        if not lines[i].strip().startswith("//"):
            i += 1
            continue
        # collect the contiguous comment block
        j = i
        block = []
        while j < len(lines) and lines[j].strip().startswith("//"):
            block.append(lines[j])
            j += 1
        head = None
        for ln in block:
            mm = ADDR_RE.search(ln)
            if mm:
                if ("insn" in ln) or ("GUESS" in ln):
                    head = int(mm.group(1), 16)
                break
        if head is not None:
            base = brace_depth(lines, 0, j)
            depth = base
            blob = " ".join(block).lower()
            for k in range(j, min(len(lines), j + 60)):
                if depth == base:
                    name = function_def(lines, k)
                    if name and name_matches(blob, name):
                        out.append((head, name, f"{rel}:{k + 1}", "RECONSTRUCTED"))
                        break
                depth += lines[k].count("{") - lines[k].count("}")
        i = j
    return out


UNPAIRED_RE = re.compile(
    r"//\s*(0x18[0-9a-fA-F]{6,})\s+(\S.*?)\s+(src/[\w./+-]+:\d+)\s*$")


def find_present_unpaired(path: str):
    """Yield PRESENT-BUT-UNPAIRED rows: addr, src/file:line, evidence."""
    rel = os.path.relpath(path, REPO)
    out = []
    for i, line in enumerate(open(path, encoding="utf-8")):
        m = UNPAIRED_RE.search(line.rstrip())
        if not m:
            continue
        out.append((int(m.group(1), 16), m.group(3),
                    f"{rel}:{i + 1} claims {m.group(2).strip()} -> {m.group(3)}",
                    "PRESENT-UNPAIRED"))
    return out


def collect(src_dir: str):
    rows = []
    for root, _dirs, files in os.walk(src_dir):
        for fn in sorted(files):
            if not fn.endswith((".cpp", ".h", ".hpp")):
                continue
            p = os.path.join(root, fn)
            if fn.startswith("Missing"):
                rows.extend(find_declarations(p) if fn.endswith(".cpp") else [])
            rows.extend(find_present_unpaired(p))
    # de-duplicate on (addr, symbol), keeping the first evidence
    seen, out = set(), []
    for addr, sym, ev, kind in rows:
        key = (addr, sym)
        if key in seen:
            continue
        seen.add(key)
        out.append({"orig_addr": hex(addr), "our_symbol_or_source_location": sym,
                    "evidence": ev, "round": round_of(ev.split(":")[0]),
                    "kind": kind})
    out.sort(key=lambda r: (int(r["orig_addr"], 16), r["kind"]))
    return out


def write_rows(rows, out_path):
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=["orig_addr",
                                           "our_symbol_or_source_location",
                                           "evidence", "round", "kind"])
        w.writeheader()
        w.writerows(rows)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", default=os.path.join(REPO, "src"))
    ap.add_argument("--out", default=os.path.join(REPO,
                                                 "recon/declared-mappings.csv"))
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()

    rows = collect(args.src)
    if args.self_test:
        ok = True
        need = {"0x180054d70": "ShutdownCaptionStore",
                "0x180013f20": "ScheduleDelayedTask",
                "0x1800107a0": None}
        by_addr = {}
        for r in rows:
            by_addr.setdefault(int(r["orig_addr"], 16), []).append(r)
        for a, sym in need.items():
            hit = by_addr.get(int(a, 16))
            if not hit:
                print(f"FAIL: {a} not extracted"); ok = False; continue
            if sym and not any(sym in r["our_symbol_or_source_location"] for r in hit):
                print(f"FAIL: {a} symbol {sym} not found in {[r['our_symbol_or_source_location'] for r in hit]}")
                ok = False
            else:
                print(f"PASS: {a} -> {[r['our_symbol_or_source_location'] for r in hit]} "
                      f"({hit[0]['evidence']})")
        kinds = {r["kind"] for r in rows}
        print(f"self-test: {len(rows)} rows, kinds={sorted(kinds)} "
              f"-> {'PASS' if ok else 'FAIL'}")
        return 0 if ok else 1

    write_rows(rows, args.out)
    print(f"wrote {args.out}: {len(rows)} declarations")
    from collections import Counter
    print(Counter(r["kind"] for r in rows))
    print(Counter(r["round"] for r in rows))
    return 0


if __name__ == "__main__":
    sys.exit(main())
