#!/usr/bin/env python3
"""D1: triage the real MISSING functions into PRESENT-UNPAIRED / ABSENT-LIBRARY /
ABSENT-PLUGIN / UNCERTAIN.

Reuses tools/parity.py's inventory + normalisation (via tools/match.py's
build_side) and searches the *_entire_* rebuild function inventory for a
normalised-token match, not just the pairs the identity matcher produced.

Output: build/recon/missing-triage.csv  (+ a copy under recon/ for the branch)
"""
from __future__ import annotations

import csv
import json
import os
import sys
import time
from collections import Counter, defaultdict
from difflib import SequenceMatcher

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))  # SexLabPPrism-re
sys.path.insert(0, os.path.join(ROOT, "wt", "triage", "tools"))
import match  # noqa: E402  (imports parity from same dir)

ORIG = os.path.join(ROOT, "artifacts/SexLabPPrism.dll")
NEW = os.path.join(ROOT, "artifacts/rebuild/parity-r4.dll")
IDENT = os.path.join(ROOT, "build/recon/matching/per-function.json")
AUDIT = os.path.join(ROOT, "build/recon/missing-classified.csv")
SYMBOLS = os.path.join(ROOT, "recon/symbols.csv")
OUT = os.path.join(ROOT, "build/recon/missing-triage.csv")
OUT_COPY = os.path.abspath(os.path.join(HERE, "..", "recon", "missing-triage.csv"))

R_THRESHOLD = 0.95
LEN_LO, LEN_HI = 0.75, 1.30
JACCARD_SCREEN = 0.30


def load_symbols(path):
    out = {}
    for row in csv.DictReader(open(path, encoding="utf-8")):
        if row.get("name"):
            out[int(row["address"], 16)] = row["name"]
    return out


def tok_counter(tokens):
    return Counter(tokens)


def main():
    t0 = time.time()
    # --- identity verdicts + the real (pdata / unwind-entry) function set ---
    d = json.load(open(IDENT))
    verdict = {int(f["orig_addr"], 16): f["verdict"] for f in d["functions"]}
    names = {int(f["orig_addr"], 16): f["name"] for f in d["functions"]}

    audit = {}
    for r in csv.DictReader(open(AUDIT, encoding="utf-8")):
        audit[int(r["address"], 16)] = r
    symbols = load_symbols(SYMBOLS)

    pe, fo, _, _ = match.build_side(ORIG)
    _, fn, _, _ = match.build_side(NEW)
    tier_map, _tn = match.compute_tiers(
        pe, fo, root=os.path.dirname(os.path.dirname(ORIG)))
    tier_basis = {f["addr"]: f.get("basis", "?") for f in fo}
    pdata = {b for b, e, _ in pe.parse_pdata() if e > b}
    print(f"orig {len(fo)} funcs, {len(pdata)} real (pdata); rebuild {len(fn)} funcs; "
          f"build {time.time()-t0:.0f}s")

    real_missing = [f for f in fo
                    if f["addr"] in pdata and verdict.get(f["addr"]) == "MISSING"]
    print(f"real MISSING = {len(real_missing)} "
          f"({sum(len(f['tokens']) for f in real_missing)} insn)")

    # --- rebuild index ---
    by_len = defaultdict(list)
    exact = defaultdict(list)
    for j, g in enumerate(fn):
        n = len(g["tokens"])
        by_len[n].append(j)
        exact[tuple(g["tokens"])].append(j)
    cnt = [tok_counter(g["tokens"]) for g in fn]
    lengths = sorted(by_len)

    def length_window(n):
        import bisect
        lo = bisect.bisect_left(lengths, int(n * LEN_LO))
        hi = bisect.bisect_right(lengths, int(n * LEN_HI) + 1)
        for k in lengths[lo:hi]:
            yield from by_len[k]

    def jaccard(a, b):
        inter = sum((a & b).values())
        tot = sum(a.values()) + sum(b.values()) - inter
        return inter / tot if tot else 0.0

    rows = []
    for f in real_missing:
        toks = f["tokens"]
        te = tuple(toks)
        best = (0.0, None)
        ge = 0
        if te in exact:
            cands = exact[te]
            best = (1.0, cands[0])
            ge = len(cands)
        else:
            screened = [j for j in length_window(len(toks))
                        if jaccard(cnt[j], tok_counter(toks)) >= JACCARD_SCREEN]
            for j in screened:
                r = SequenceMatcher(None, toks, fn[j]["tokens"], autojunk=False).ratio()
                if r >= R_THRESHOLD:
                    ge += 1
                if r > best[0]:
                    best = (r, j)
        addr = f["addr"]
        nm = names.get(addr) or symbols.get(addr) or ""
        if best[1] is not None and best[0] >= R_THRESHOLD:
            j = best[1]
            paired_to = None
            cls = "PRESENT-UNPAIRED"
            ev = (f"rebuild 0x{fn[j]['addr']:x} ratio={best[0]:.3f} "
                  f"({len(fn[j]['tokens'])} insn); {ge} rebuild func(s) >=0.95")
            if ge > 4:
                ev += " [generic-template risk]"
            action = "improve pairing (identity anchors); do NOT implement"
        else:
            a = audit.get(addr)
            acls = a["classification"] if a else "uncertain"
            aev = (a["evidence"] if a else "no distinctive reference")[:300]
            tier = tier_map.get(addr, "library")
            basis = tier_basis.get(addr, "?")
            if acls == "library" or (tier == "library" and "import" in aev):
                cls = "ABSENT-LIBRARY"
                action = "build-config / template-shape, not hand-written body"
            elif acls == "plugin" or tier == "plugin":
                cls = "ABSENT-PLUGIN"
                action = "implement (plugin-owned, no rebuild match)"
            else:
                cls = "UNCERTAIN"
                action = "needs manual adjudication"
            ev = (f"{aev} | tier={tier} ({basis}) | best cross-all ratio="
                  f"{best[0]:.3f}" + (f" (0x{fn[best[1]]['addr']:x})"
                                      if best[1] is not None else ""))
        rows.append({
            "address": hex(addr), "insn": len(toks), "name": nm, "class": cls,
            "evidence": ev, "action": action,
        })

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=["address", "insn", "name", "class",
                                           "evidence", "action"])
        w.writeheader()
        w.writerows(rows)
    with open(OUT_COPY, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=["address", "insn", "name", "class",
                                           "evidence", "action"])
        w.writeheader()
        w.writerows(rows)

    by = Counter(r["class"] for r in rows)
    ins = Counter()
    for r in rows:
        ins[r["class"]] += r["insn"]
    print("class,funcs,insn")
    for c in ("PRESENT-UNPAIRED", "ABSENT-LIBRARY", "ABSENT-PLUGIN", "UNCERTAIN"):
        print(f"  {c:16s} {by.get(c,0):4d} funcs  {ins.get(c,0):6d} insn")
    print("top ABSENT-PLUGIN by insn:")
    for r in sorted((r for r in rows if r["class"] == "ABSENT-PLUGIN"),
                    key=lambda r: -r["insn"])[:20]:
        print(f"  {r['address']:>12s} {r['insn']:4d} {r['name'][:40]:40s} {r['evidence'][:80]}")
    print(f"total {len(rows)} funcs {sum(r['insn'] for r in rows)} insn; "
          f"wrote {OUT}")


if __name__ == "__main__":
    main()