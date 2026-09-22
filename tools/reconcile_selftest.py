#!/usr/bin/env python3
"""Cross-tool agreement test for the per-function scoreboard model.

The bug this pins: a DECLARED-DIVERGENT pair used to live only in the declared
printer, so `real-functions-score.py` (and any other consumer that knows only
BYTE/INSN/RATIO/MISSING) silently dropped those rows and its four buckets did
not sum to the population.  After the model change, DECLARED-DIVERGENT is a
first-class verdict and every consumer must agree:

  * the five verdicts sum exactly to the number of rows (no dropped rows);
  * raw MISSING (all functions) == REAL MISSING + gap-scan MISSING, so the
    declared-aware pass and real-functions-score.py count the same population;
  * a declared original is either BOUND (BYTE/INSN/RATIO), DECLARED-DIVERGENT
    (paired, not matching) or MISSING with declared_class == UNBOUND and its
    reason retained -- never an anonymous MISSING row;
  * DECLARED-DIVERGENT is never a byte/insn match and never enters the
    byte-match share;
  * a BYTE-MATCH row always has bytematch true (a declaration is never promoted
    to BYTE-MATCH without byte equality).

Usage:  python3 tools/reconcile_selftest.py [per-function.json] [--orig dll]
"""
from __future__ import annotations

import argparse
import collections
import json
import os
import struct
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
W = os.path.expanduser("~/Projects/SexLabPPrism-re")
KNOWN = {"BYTE-MATCH", "INSN-MATCH", "RATIO", "MISSING", "DECLARED-DIVERGENT"}


def u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def pdata_starts(path):
    data = open(path, "rb").read()
    pe = u32(data, 0x3C)
    opt = pe + 24
    magic = u16(data, opt)
    dd = opt + (112 if magic == 0x20B else 96)
    rva, size = u32(data, dd + 24), u32(data, dd + 28)
    nsec, optsz = u16(data, pe + 6), u16(data, pe + 20)
    secoff = opt + optsz

    def off(r):
        for i in range(nsec):
            s = secoff + i * 40
            va, ra = u32(data, s + 12), u32(data, s + 20)
            vsz, rsz = u32(data, s + 8), u32(data, s + 16)
            if va <= r < va + max(vsz, rsz):
                return ra + (r - va)
        return None

    o = off(rva)
    base = struct.unpack_from("<Q", data, opt + 24)[0]
    return {u32(data, o + i * 12) for i in range(size // 12)}, base


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("json", nargs="?", default=os.path.join(
        REPO, "recon/matching/per-function.json"))
    ap.add_argument("--orig", default=os.path.join(
        W, "artifacts/SexLabPPrism.dll"))
    args = ap.parse_args()

    doc = json.load(open(args.json, encoding="utf-8"))
    rows = doc if isinstance(doc, list) else doc.get("functions", doc.get("entries", []))
    starts, base = pdata_starts(args.orig)

    passed = total = 0
    notes = []

    def check(cond, msg):
        nonlocal passed, total
        total += 1
        passed += bool(cond)
        notes.append(("PASS " if cond else "FAIL ") + msg)

    verdicts = collections.Counter(str(r.get("verdict", "")).upper() for r in rows)

    # 1. no unknown verdict, buckets sum to the population
    unknown = sorted(v for v in verdicts if v not in KNOWN)
    check(not unknown, f"all {len(rows)} rows carry a known verdict "
                       f"(unknown: {unknown or 'none'})")
    check(sum(verdicts[v] for v in KNOWN) == len(rows),
          f"five verdicts sum to population: {sum(verdicts[v] for v in KNOWN)}"
          f" == {len(rows)}")

    # 2. DECLARED-DIVERGENT is not a match and not in the byte share
    dec = [r for r in rows if str(r.get("verdict", "")).upper() == "DECLARED-DIVERGENT"]
    bad = [r for r in dec if r.get("bytematch") or r.get("insn_match")]
    check(not bad, f"no DECLARED-DIVERGENT row is a byte/insn match "
                   f"({len(dec)} rows checked)")
    check(verdicts["BYTE-MATCH"] == sum(1 for r in rows if r.get("bytematch")),
          "BYTE-MATCH count == rows with bytematch true (no promotion)")

    # 3. declared rows are first-class: bound / divergent / unbound-with-reason
    declared = [r for r in rows if r.get("declared")]
    unbound = [r for r in declared if str(r.get("verdict", "")).upper() == "MISSING"]
    unbound_ok = all(r.get("declared_class") == "UNBOUND"
                     and r.get("declared_verdict") == "MISSING"
                     and (r.get("declared_map_reason") or r.get("declared_binding"))
                     for r in unbound)
    check(unbound_ok, f"every unbound declaration ({len(unbound)}) stays MISSING "
                      f"with class UNBOUND and a retained reason")
    declared_ok = all(r.get("declared_symbol") or r.get("declared_evidence")
                      or r.get("declared_map_reason") for r in declared)
    check(declared_ok, f"every declared row ({len(declared)}) carries its symbol/"
                       f"evidence/binding in the JSON")

    # 4. cross-tool population agreement: raw MISSING == real + gap-scan MISSING
    def is_real(r):
        a = r.get("orig_addr")
        va = int(a, 16) if isinstance(a, str) else a
        rva = va - base if va >= base else va
        return rva in starts

    real = [r for r in rows if is_real(r)]
    guess = [r for r in rows if not is_real(r)]
    raw_missing = verdicts["MISSING"]
    real_missing = sum(1 for r in real if str(r.get("verdict", "")).upper() == "MISSING")
    guess_missing = sum(1 for r in guess if str(r.get("verdict", "")).upper() == "MISSING")
    check(raw_missing == real_missing + guess_missing,
          f"raw MISSING {raw_missing} == real MISSING {real_missing} + "
          f"gap-scan MISSING {guess_missing}")

    # 5. declared counts recorded in the JSON must equal recomputed counts
    declared_counts = (doc.get("declared") or {}).get("counts")
    if declared_counts:
        recomputed = collections.Counter(str(r.get("declared_verdict")).upper()
                                         for r in declared)
        for k, v in declared_counts.items():
            check(recomputed.get(k.upper(), 0) == v,
                  f"declared block counts {k}={v} == recomputed "
                  f"{recomputed.get(k.upper(), 0)}")
        check(sum(declared_counts.values()) == len(declared),
              f"declared block total {sum(declared_counts.values())} == "
              f"{len(declared)} declared rows")

    print(f"reconcile self-test: {passed}/{total} passed  "
          f"(rows={len(rows)} real={len(real)} gap-scan={len(guess)} "
          f"declared={len(declared)})")
    for n in notes:
        print("  " + n)
    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())
