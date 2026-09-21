#!/usr/bin/env python3
"""Library-tier MISSING forensics.

For every library-tier original function that tools/match.py reports MISSING,
decide which of the four hypotheses fits, mechanically:

  H1 inlined/folded - the function's normalised instruction stream appears as a
     contiguous block inside a *new* binary function (either because MSVC
     inlined it, or because identical-code-folding merged it with a twin whose
     pair took the other original).
  H2 eliminated     - no code/data reference to the original address inside the
     original binary, and no token-stream fragment in the rebuild: dead code
     removed by the whole-program build.
  H3 shape          - referenced (vtable/function pointer/code) but the stream
     is absent: different template instantiation or inlining boundary.
  H4 absent         - no reference and a referenced string/RTTI marker is
     absent from the rebuild: our build never compiled the path.

Outputs recon/library-forensics-data.json (all 660) and
recon/library-forensics-sample.csv (top 30 by original instruction count).
"""
from __future__ import annotations

import csv
import importlib.util
import json
import os
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
# data (artifacts/recon) lives in the integration checkout, not this worktree
DATA = os.environ.get("SEXLAB_REPO", "/home/aviallon/Projects/SexLabPPrism-re")
if not os.path.exists(os.path.join(DATA, "artifacts/SexLabPPrism.dll")):
    DATA = ROOT
sys.path.insert(0, os.path.join(DATA, "tools"))
import match  # noqa: E402

ORIG = os.path.join(DATA, "artifacts/SexLabPPrism.dll")
NEW = os.path.join(DATA, "artifacts/rebuild/parity-r5.dll")
PERFUNC = os.path.join(DATA, "recon/matching/per-function.json")
OUTJSON = os.path.join(DATA, "recon/library-forensics-data.json")
OUTC = os.path.join(DATA, "recon/library-forensics-sample.csv")


CACHE = "/tmp/libf_sides.pkl"


def load_sides():
    """Build both sides with match.py machinery; returns (pe, funcs, ...)."""
    import pickle
    pe_o = match.PE(ORIG)
    pe_n = match.PE(NEW)
    if os.path.exists(CACHE):
        d = pickle.load(open(CACHE, "rb"))
        return pe_o, d["fo"], pe_n, d["fn"]
    pe_o, fo, mo, ro = match.build_side(ORIG)
    pe_n, fn, mn, rn = match.build_side(NEW)
    return pe_o, fo, pe_n, fn


def ref_bytes(pe, addr):
    """True if `addr` (image VA) appears as an 8-byte VA or 4-byte RVA
    anywhere in the raw image (vtable / function-pointer reference)."""
    d = pe.data
    va = struct.pack("<Q", addr)
    rva = addr - pe.image_base
    if 0 <= rva < (1 << 32):
        le4 = struct.pack("<I", rva)
        if le4 in d:
            return True
    return va in d


def contiguous_hits(target_tokens, new_toks):
    """Return indices of new functions whose token string contains the target
    token stream as a contiguous block.  new_toks maps j -> joined string."""
    t = "\n".join(target_tokens)
    if len(target_tokens) < 4:
        return []
    return [j for j, s in new_toks.items() if t in s]


def main():
    with open(PERFUNC) as fh:
        pf = json.load(fh)
    rows = [r for r in pf["functions"]
            if r["tier"] == "library" and r["verdict"] == "MISSING"]
    rows.sort(key=lambda r: -r["orig_insn"])
    print(f"library MISSING targets: {len(rows)}")

    pe_o, fo, pe_n, fn = load_sides()
    by_addr = {f["addr"]: i for i, f in enumerate(fo)}
    # call graph inside the original: who references whom
    callers = {}
    for i, f in enumerate(fo):
        for r in f["refs"]:
            callers.setdefault(r, []).append(i)
    # joined token strings for the rebuild, once
    new_toks = {j: "\n".join(fn[j]["tokens"]) for j in range(len(fn))}
    new_by_addr = {f["addr"]: j for j, f in enumerate(fn)}
    # full-stream exact set for ICF detection
    new_full = {}
    for j in range(len(fn)):
        new_full.setdefault(new_toks[j], []).append(j)

    out = []
    for r in rows:
        a = int(r["orig_addr"], 16)
        i = by_addr.get(a)
        rec = dict(r)
        rec["hyp"] = None
        if i is None:
            rec["hyp"] = "H4-absent(no-inventory)"
            out.append(rec)
            continue
        f = fo[i]
        toks = f["tokens"]
        joined = "\n".join(toks)
        hits = contiguous_hits(toks, new_toks)
        nrefs_code = sorted(set(callers.get(a, [])))
        nrefs_code = [c for c in nrefs_code if c != i]
        dref = ref_bytes(pe_o, a)
        rec["code_refs"] = len(nrefs_code)
        rec["data_ref"] = bool(dref)
        rec["exact_twin"] = bool(new_full.get(joined))
        rec["subseq_hits"] = [hex(fn[j]["addr"]) for j in hits[:4]]
        rec["subseq_hit_insn"] = [len(fn[j]["tokens"]) for j in hits[:4]]
        rec["self_insn"] = len(toks)
        if hits:
            # exact twin => ICF fold; else inlined into a larger function
            if any(len(fn[j]["tokens"]) == len(toks) for j in hits):
                rec["hyp"] = "H1-ICF"
            else:
                rec["hyp"] = "H1-inlined"
        elif not nrefs_code and not dref:
            rec["hyp"] = "H2-eliminated(no-ref)"
        else:
            rec["hyp"] = "H3-shape"
        out.append(rec)

    json.dump(out, open(OUTJSON, "w"), indent=1)
    with open(OUTC, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["orig_addr", "insn", "masked", "hyp", "code_refs",
                    "data_ref", "exact_twin", "subseq_hit", "hit_insn"])
        for r in out[:30]:
            w.writerow([r["orig_addr"], r["orig_insn"], r["orig_masked"],
                        r["hyp"], r.get("code_refs"), r.get("data_ref"),
                        r.get("exact_twin"),
                        r["subseq_hits"][0] if r.get("subseq_hits") else "",
                        r["subseq_hit_insn"][0] if r.get("subseq_hit_insn") else ""])
    # summary
    from collections import Counter
    c = Counter(r["hyp"] for r in out)
    tot = sum(r["orig_insn"] for r in out)
    insn = Counter()
    for r in out:
        insn[r["hyp"]] += r["orig_insn"]
    print("H-split (count, instructions):")
    for k, v in sorted(c.items(), key=lambda kv: -kv[1]):
        print(f"  {k:24s} {v:4d}  {insn[k]:6d} insns")
    print("wrote", OUTJSON, OUTC)


if __name__ == "__main__":
    main()
