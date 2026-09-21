#!/usr/bin/env python3
"""Self-test for the declared-pair extractor and the DECLARED-DIVERGENT verdict.

Negative cases (required by the round-12 brief):

  A. a declared address whose target function is absent from our binary must
     stay MISSING (never invented);
  B. a declaration on a function that is already BYTE-MATCH must not change that
     verdict (a declaration must not turn a BYTE-MATCH into something else);
  C. a WRONG declaration must surface as a low ratio under
     DECLARED-DIVERGENT, not be hidden as a match;
  D. the extractor itself must recover known declarations from the sources
     (tools/extract_declared.py --self-test).

Usage:
  python3 tools/declared_selftest.py --orig <dll> --new <dll>
"""
from __future__ import annotations

import argparse
import collections
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import match as M  # noqa: E402

REPO = os.path.dirname(HERE)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--orig", default=os.path.join(
        REPO, "artifacts/SexLabPPrism.dll"))
    ap.add_argument("--new", default=os.path.join(
        REPO, "artifacts/rebuild/parity-r17missing2.dll"))
    args = ap.parse_args()

    passed = total = 0
    notes = []

    def check(cond, msg):
        nonlocal passed, total
        total += 1
        passed += bool(cond)
        notes.append(("PASS " if cond else "FAIL ") + msg)

    # D. extractor self-test (subprocess, so it is the shipped CLI path)
    r = subprocess.run([sys.executable, os.path.join(HERE, "extract_declared.py"),
                        "--self-test"], capture_output=True, text=True)
    check(r.returncode == 0, "extractor self-test exit 0: "
          + r.stdout.strip().splitlines()[-1])

    pe, fo, _, _ = M.build_side(args.orig)
    _, fn, _, _ = M.build_side(args.new)
    pdata = {b for b, e, _ in pe.parse_pdata() if e > b}
    aidx = {f["addr"]: i for i, f in enumerate(fo)}

    def fresh():
        pairs, unmA, unmB = M.pair(fo, fn)
        M.verdicts(fo, fn, pairs)
        return pairs, unmA, unmB

    # ---- case A: declared but no target function anywhere ------------------
    base_pairs, base_unmA, base_unmB = fresh()
    vmap = {fo[p["i"]]["addr"]: p["verdict"] for p in base_pairs}
    miss_real = [f["addr"] for f in fo
                 if f["addr"] in pdata and vmap.get(f["addr"], M.MISSING) == M.MISSING]
    a_addr = sorted(miss_real)[0]
    dec = {a_addr: {"orig_addr": hex(a_addr), "our_symbol_or_source_location":
                    "NoSuchBody", "evidence": "selftest:A", "kind": "test",
                    "our_new_addr": "0x1800fff0"}}
    pairs, unmA, unmB, rows = M.apply_declared(fo, fn, list(base_pairs),
                                               base_unmA, base_unmB, dec, 0.5)
    check(rows[0]["verdict"] == M.MISSING,
          f"A. absent target: {hex(a_addr)} stays MISSING "
          f"(verdict={rows[0]['verdict']})")

    # ---- case B: declaration must not downgrade BYTE-MATCH ------------------
    byte_real = [f["addr"] for f in fo
                 if f["addr"] in pdata and vmap.get(f["addr"]) == M.BYTE]
    b_addr = sorted(byte_real)[0]
    dec = {b_addr: {"orig_addr": hex(b_addr), "our_symbol_or_source_location":
                    "AlreadyMatched", "evidence": "selftest:B", "kind": "test"}}
    pairs, unmA, unmB, rows = M.apply_declared(fo, fn, list(base_pairs),
                                               base_unmA, base_unmB, dec, 0.5)
    check(rows[0]["verdict"] == M.BYTE and rows[0]["bytematch"] == 1,
          f"B. no downgrade: {hex(b_addr)} BYTE-MATCH -> {rows[0]['verdict']} "
          f"bytematch={rows[0]['bytematch']}")

    # ---- case C: wrong declaration is visible as a low ratio ---------------
    # pin a MISSING original to a rebuild function that is not its counterpart
    other = next(j for j in range(len(fn)) if len(fn[j]["tokens"]) >= 3
                 and fn[j]["tokens"] != fo[aidx[a_addr]]["tokens"])
    dec = {a_addr: {"orig_addr": hex(a_addr), "our_symbol_or_source_location":
                    "WrongBody", "evidence": "selftest:C", "kind": "test",
                    "our_new_addr": hex(fn[other]["addr"])}}
    pairs, unmA, unmB, rows = M.apply_declared(fo, fn, list(base_pairs),
                                               base_unmA, base_unmB, dec, 0.5)
    check(rows[0]["verdict"] != M.BYTE and rows[0]["bytematch"] == 0
          and rows[0]["ratio"] < 0.95,
          f"C. wrong declaration not BYTE-MATCH: verdict={rows[0]['verdict']} "
          f"ratio={rows[0]['ratio']:.4f} (pin={hex(fn[other]['addr'])}, not "
          f"{hex(a_addr)}'s counterpart)")

    # ---- case E: declared+absent body (no pin) stays MISSING, not seeded ----
    # an address declared with an impossible symbol and no structural candidate
    dec = {a_addr: {"orig_addr": hex(a_addr), "our_symbol_or_source_location":
                    "NoSuchBody", "evidence": "selftest:E", "kind": "test"}}
    pairs, unmA, unmB, rows = M.apply_declared(fo, fn, list(base_pairs),
                                               base_unmA, base_unmB, dec, 0.99)
    check(rows[0]["verdict"] == M.MISSING,
          f"E. min-ratio gate: {hex(a_addr)} stays MISSING at min_ratio=0.99")

    print(f"declared self-test: {passed}/{total} passed")
    for n in notes:
        print("  " + n)
    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())
