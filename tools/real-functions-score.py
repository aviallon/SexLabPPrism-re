#!/usr/bin/env python3
"""Restrict the matching scoreboard to REAL original functions (those with an
x64 unwind entry), which is the only defensible denominator for a parity claim.
Gap-scan "functions" are bytes inside .text with no unwind record; they inflate
both the numerator and the denominator, so they are reported separately.

DECLARED-DIVERGENT is a FIRST-CLASS verdict here exactly as it is in match.py:
a declared original whose body did not match is neither BYTE/INSN/RATIO nor
MISSING, and counts toward none of them (nor the byte-match share).  It is
printed as its own bucket so the five verdicts always sum to the population.

Usage:
  python3 tools/real-functions-score.py [per-function.json]
"""
import collections
import json
import pathlib
import struct
import sys

W = pathlib.Path.home() / "Projects/SexLabPPrism-re"
JSON = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 \
    else W / "recon/matching/per-function.json"


def u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def pdata_starts(data):
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


base_addr, = [None]
data = (W / "artifacts/SexLabPPrism.dll").read_bytes()
starts, base = pdata_starts(data)
doc = json.loads(JSON.read_text())
entries = doc if isinstance(doc, list) else doc.get("functions", doc.get("entries", []))

real, guess = [], []
for e in entries:
    a = e.get("orig_addr")
    va = int(a, 16) if isinstance(a, str) else a
    rva = va - base if va >= base else va
    (real if rva in starts else guess).append(e)

ORDER = ["BYTE-MATCH", "INSN-MATCH", "RATIO", "MISSING", "DECLARED-DIVERGENT"]


def bucket_insn(rows, v):
    return sum(int(r.get("orig_insn") or 0) for r in rows
               if str(r["verdict"]).upper() == v)


def summarise(name, rows):
    counts = collections.Counter(str(r["verdict"]).upper() for r in rows)
    insn = sum(int(r.get("orig_insn") or 0) for r in rows)
    matched_insn = bucket_insn(rows, "BYTE-MATCH")
    print(f"\n{name}: {len(rows):,} functions, {insn:,} instructions")
    for v in ORDER:
        c = counts.get(v, 0)
        ci = bucket_insn(rows, v)
        print(f"  {v:<18} {c:>5,} funcs ({c / len(rows) * 100:5.1f}%)  {ci:>7,} insn")
    unaccounted = len(rows) - sum(counts.get(v, 0) for v in ORDER)
    if unaccounted:
        print(f"  !! {unaccounted} rows carry an unknown verdict "
              f"(buckets do not sum to the population)")
    print(f"  byte-match share: {counts.get('BYTE-MATCH', 0) / len(rows) * 100:.2f}% of funcs, "
          f"{matched_insn / insn * 100:.2f}% of instructions")
    for tier in ("plugin", "library"):
        tr = [r for r in rows if r.get("tier") == tier]
        if not tr:
            continue
        tc = collections.Counter(str(r["verdict"]).upper() for r in tr)
        ti = sum(int(r.get("orig_insn") or 0) for r in tr)
        tb = bucket_insn(tr, "BYTE-MATCH")
        print(f"    {tier:<8} {len(tr):>5,} funcs  BYTE {tc.get('BYTE-MATCH', 0):>5,} "
              f"({tc.get('BYTE-MATCH', 0) / len(tr) * 100:5.1f}%)  insn {tb / ti * 100:5.1f}%")
    return counts


print(f"original .pdata function starts: {len(starts):,}")
print(f"per-function JSON: {JSON}")
rc = summarise("REAL functions (unwind entry, the defensible denominator)", real)
gc = summarise("gap-scan pseudo-functions (no unwind record - measurement noise)", guess)

# Cross-tool invariant: the raw (all-functions) MISSING in match.py must equal
# REAL MISSING + gap-scan MISSING, and both splits must sum to their population.
assert sum(rc.get(v, 0) for v in ORDER) == len(real), "REAL buckets do not sum"
assert sum(gc.get(v, 0) for v in ORDER) == len(guess), "gap-scan buckets do not sum"

# what would the target require
missing_real = [r for r in real if str(r["verdict"]).upper() != "BYTE-MATCH"]
print(f"\nTo reach the user's target on REAL functions (>98% byte-match, none below 90%):")
print(f"  currently not byte-matched: {len(missing_real):,} of {len(real):,} "
      f"({len(missing_real) / len(real) * 100:.1f}%)")
print(f"  allowed failures at 98%: {int(len(real) * 0.02):,}")
missing_insn = sum(int(r.get("orig_insn") or 0) for r in missing_real)
print(f"  their instruction volume: {missing_insn:,}")
