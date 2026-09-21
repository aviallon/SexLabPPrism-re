#!/usr/bin/env python3
"""H3 (shape/instantiation) diagnostics for the library tier.

Classifies the shape differences between the original DLL and a parity rebuild
WITHOUT re-implementing any PE parsing: RTTI comes from tools/pe_rtti.py, and
this script only compares the two reconstructed RTTI inventories plus the
printable strings that MSVC emits for std::function lambda wrappers
(`std::_Func_impl_no_alloc<...>` type names appear as `__FUNCSIG__` literals).

Usage:
  python3 tools/h3-shapes.py \
      --orig artifacts/SexLabPPrism.dll \
      --new  artifacts/rebuild/parity-r6.dll \
      --rtti-orig /tmp/rttio/recon/rtti.json \
      --rtti-new  /tmp/rtti6/recon/rtti.json \
      --out recon/h3-shapes-data.json

Everything is evidence: the printed sets are exactly what the two binaries
contain.  No claim of fixability is made here.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys


def printable_strings(path, minlen=8):
    data = open(path, "rb").read()
    pat = re.compile(rb"[\x20-\x7e]{%d,}" % minlen)
    return {m.group().decode("latin1") for m in pat.finditer(data)}


def recover_rtti(path, rtti_json):
    if rtti_json and os.path.exists(rtti_json):
        return json.load(open(rtti_json))
    here = os.path.dirname(os.path.abspath(__file__))
    sys.path.insert(0, here)
    import parity_names  # noqa: E402
    return parity_names.recover_rtti(path)


def wrapper_names(strings):
    return {s for s in strings if "_Func_impl_no_alloc" in s}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--orig", required=True)
    ap.add_argument("--new", required=True)
    ap.add_argument("--rtti-orig")
    ap.add_argument("--rtti-new")
    ap.add_argument("--out")
    a = ap.parse_args()

    ro = recover_rtti(a.orig, a.rtti_orig)
    rn = recover_rtti(a.new, a.rtti_new)
    so, sn = set(ro.get("type_descriptors", {}).values()), set(rn.get("type_descriptors", {}).values())

    po, pn = printable_strings(a.orig), printable_strings(a.new)
    wo, wn = wrapper_names(po), wrapper_names(pn)

    print(f"type descriptors: orig={len(so)} new={len(sn)} only_orig={len(so - sn)} only_new={len(sn - so)}")
    for t in sorted(so - sn):
        print(f"  TD only orig: {t}")
    for t in sorted(sn - so):
        print(f"  TD only new : {t}")

    print(f"_Func_impl_no_alloc wrappers: orig={len(wo)} new={len(wn)} only_orig={len(wo - wn)} only_new={len(wn - wo)}")
    for s in sorted(wo - wn):
        print(f"  wrapper only orig: {s}")
    for s in sorted(wn - wo):
        print(f"  wrapper only new : {s}")

    if a.out:
        json.dump({
            "type_descriptors_only_orig": sorted(so - sn),
            "type_descriptors_only_new": sorted(sn - so),
            "wrappers_only_orig": sorted(wo - wn),
            "wrappers_only_new": sorted(wn - wo),
        }, open(a.out, "w"), indent=1)
        print(f"wrote {a.out}")


if __name__ == "__main__":
    main()
