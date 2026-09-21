#!/usr/bin/env python3
"""Classify every MISSING original function by WHY it is missing.

The MISSING bucket conflates three different realities:

  * ABSENT          - no trace of the original's code anywhere in our binary
                      (never implemented, or eliminated by the compiler;
                      a source-string check hints at /OPT:REF).
  * PRESENT-INFOLDED- our binary DOES contain code that provably serves the
                      original function, but it lives inside a larger function
                      X that is already paired to a DIFFERENT original Y.
                      Evidence: a distinctive anchor of the missing original
                      (a string literal only it references, an RTTI name only it
                      carries, or a callee only it calls) is referenced from X,
                      and X is paired to Y != missing.
  * PRESENT-UNPAIRED- the anchor appears in our binary, but in a rebuild
                      function that the identity pairer did not pair to
                      anything.  The code exists; the pairing failed.
  * UNKNOWN          - the missing original has no distinctive anchor at all,
                      so the tool cannot decide.  Reported, never guessed.

This is a MEASUREMENT layer: it never adds, removes or changes a pair.  The
pairing is produced by tools/pair_identity.py exactly as tools/match.py
--identity does it, and the BYTE/INSN/RATIO verdicts come from
tools/match.py's own `verdicts()`.  A PRESENT-INFOLDED verdict is therefore
*not* a match and is never counted as one.

Usage:
  python3 tools/pairfix_infolding.py \
      --orig /abs/artifacts/SexLabPPrism.dll \
      --new  /abs/artifacts/rebuild/<newest>.dll \
      --out  recon/pairfix-infolding.json \
      --report recon/pairfix-infolding.md \
      [--focus 0x1800250f0]
  python3 tools/pairfix_infolding.py --self-test
"""
from __future__ import annotations

import argparse
import collections
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import match as M        # noqa: E402  (verdicts + side construction)
import parity as P       # noqa: E402
import parity_names as N  # noqa: E402
import pair_identity as PI  # noqa: E402

MISSING = M.MISSING
ORDER = ["BYTE-MATCH", "INSN-MATCH", "RATIO",
         "PRESENT-INFOLDED", "PRESENT-UNPAIRED", "ABSENT", "UNKNOWN"]

MIN_LIT = 6          # a "distinctive" literal must be at least this long
FUZZY_PREFIX = 12    # shared prefix that binds two literals as "the same anchor"


def _lcp(a, b):
    n = 0
    for x, y in zip(a, b):
        if x != y:
            break
        n += 1
    return n


def _fuzzy_users(lit, n_lit_users):
    """Rebuild functions referencing a literal that IS this anchor.

    The original and our rebuild can store the same generated JS/format text in
    different chunks (measured: original `window.slppState(`, ours
    `window.slppState({});`), so equality is too strict.  Two literals bind when
    one contains the other or they share a >=FUZZY_PREFIX prefix.
    """
    users = set()
    for l2, addrs in n_lit_users.items():
        if len(l2) < MIN_LIT:
            continue
        if _lcp(lit, l2) >= FUZZY_PREFIX or \
                ((lit in l2 or l2 in lit) and min(len(lit), len(l2)) >= FUZZY_PREFIX):
            users |= addrs
    return users


# ---------------------------------------------------------------------------
# anchors
# ---------------------------------------------------------------------------

def _names(side, addr):
    """Normalise side.addr_to_names[addr] (set|str|None) to a sorted list."""
    v = side.addr_to_names.get(addr)
    if not v:
        return []
    if isinstance(v, str):
        return [v]
    return sorted(v)


def literal_users(side):
    """literal -> set(function addrs on this side that reference it)."""
    inv = collections.defaultdict(set)
    for a, lits in side.litrefs.items():
        for lit in lits:
            inv[lit].add(a)
    return inv


def classify_missing(fo, pairs, verdicts, o, n, src_literals):
    """Return {orig_addr: record} for every MISSING real original.

    `pairs`      : list from pair_identity.pair_indices (orig_addr/new_addr)
    `verdicts`   : {orig_addr: verdict}
    `o`,`n`      : pair_identity Side objects for original / rebuild
    `src_literals`: set of literals that occur in our source tree (hint only)
    """
    pair_of = {p["orig_addr"]: p["new_addr"] for p in pairs}
    rev_pair = {p["new_addr"]: p["orig_addr"] for p in pairs}
    paired_new = set(pair_of.values())

    o_lit_users = literal_users(o)
    n_lit_users = literal_users(n)
    # callers of each rebuild callee
    n_callers = n.callers

    out = {}
    for f in fo:
        a = f["addr"]
        if f.get("source") != "pdata" or verdicts.get(a) != MISSING:
            continue
        insn = len(f["tokens"])
        rec = {"orig_addr": a, "orig_insn": insn,
               "name": (_names(o, a) or [""])[0],
               "class": "UNKNOWN", "evidence": None}
        evidence = None

        # ---- anchor 1: unique string literal -----------------------------
        for lit in sorted(o.litrefs.get(a, ())):
            if len(lit) < MIN_LIT:
                continue
            if len(o_lit_users.get(lit, ())) != 1:
                continue                      # not distinctive
            users = _fuzzy_users(lit, n_lit_users)
            if not users:
                continue
            owner = next((x for x in users if x in paired_new), None)
            if owner is not None and rev_pair[owner] != a:
                evidence = {"anchor": "literal", "value": lit,
                            "owner": owner, "owner_paired_to": rev_pair[owner],
                            "kind": "INFOLDED"}
                break
            evidence = {"anchor": "literal", "value": lit,
                        "owner": next(iter(users)), "kind": "UNPAIRED"}
        if evidence is None:
            # register evidence even if it did not lead to a class, for ABSENT
            for lit in sorted(o.litrefs.get(a, ())):
                if len(lit) >= MIN_LIT and len(o_lit_users.get(lit, ())) == 1:
                    evidence = {"anchor": "literal", "value": lit,
                                "owner": None, "kind": "ABSENT",
                                "in_source": lit in src_literals}
                    break

        # ---- anchor 2: distinctive callee ---------------------------------
        if evidence is None or evidence.get("kind") != "INFOLDED":
            for c in sorted(o.callees.get(a, ())):
                if len(o.callers.get(c, ())) > 2:
                    continue
                na = pair_of.get(c)
                if na is None:
                    continue
                for x in sorted(n_callers.get(na, ())):
                    if x in paired_new and rev_pair[x] != a:
                        evidence = {"anchor": "callee",
                                    "value": hex(c), "owner": x,
                                    "owner_paired_to": rev_pair[x],
                                    "kind": "INFOLDED"}
                        break
                if evidence and evidence.get("kind") == "INFOLDED":
                    break

        # ---- anchor 3: unique RTTI name -----------------------------------
        if evidence is None or evidence.get("kind") != "INFOLDED":
            for nm in _names(o, a):
                if (o.kind.get(nm) or "").startswith("rtti") and \
                        len(o.name_to_addrs.get(nm, ())) <= 2:
                    for x in n.name_to_addrs.get(nm, ()):
                        if x in paired_new and rev_pair[x] != a:
                            evidence = {"anchor": "rtti", "value": nm,
                                        "owner": x,
                                        "owner_paired_to": rev_pair[x],
                                        "kind": "INFOLDED"}
                            break
                if evidence and evidence.get("kind") == "INFOLDED":
                    break

        if evidence is None:
            rec["class"] = "UNKNOWN"
        elif evidence["kind"] == "INFOLDED":
            rec["class"] = "PRESENT-INFOLDED"
        elif evidence["kind"] == "UNPAIRED":
            rec["class"] = "PRESENT-UNPAIRED"
        else:
            rec["class"] = "ABSENT"
        rec["anchors"] = {
            "lits": len(o.litrefs.get(a, ())),
            "unique_lits": sum(1 for l in o.litrefs.get(a, ())
                               if len(l) >= MIN_LIT and len(o_lit_users.get(l, ())) == 1),
            "callees": len(o.callees.get(a, ())),
            "unique_callees": sum(1 for c in o.callees.get(a, ())
                                  if len(o.callers.get(c, ())) <= 2),
        }
        if evidence:
            ev = dict(evidence)
            ev.pop("kind", None)
            if ev.get("owner") is not None:
                ev["owner_name"] = (_names(n, ev["owner"]) or [""])[0]
            rec["evidence"] = ev
        out[a] = rec
    return out


# ---------------------------------------------------------------------------
# self-test (no binary needed): the classifier's decision rules
# ---------------------------------------------------------------------------

class _FakeSide:
    def __init__(self, litrefs, callees, callers, names, kind, name_to_addrs):
        self.litrefs = litrefs
        self.callees = callees
        self.callers = callers
        self.addr_to_names = names
        self.kind = kind
        self.name_to_addrs = name_to_addrs


def self_test():
    checks = []
    # one MISSING original at 0x10, one paired owner at 0x20 (paired to 0x99)
    fo = [{"addr": 0x10, "source": "pdata", "tokens": ["a"] * 5},
          {"addr": 0x20, "source": "pdata", "tokens": ["b"] * 3},
          {"addr": 0x30, "source": "pdata", "tokens": ["c"] * 3}]
    pairs = [{"orig_addr": 0x20, "new_addr": 0x200},
             {"orig_addr": 0x30, "new_addr": 0x300}]
    verdicts = {0x10: MISSING, 0x20: "BYTE-MATCH", 0x30: "BYTE-MATCH"}

    def run(o_lit, n_lit, o_cal=None, n_cal=None, o_callers=None):
        o = _FakeSide(o_lit, o_cal or {}, o_callers or {}, {}, {}, {})
        n = _FakeSide(n_lit, {}, n_cal or {}, {}, {}, {})
        return classify_missing(fo, pairs, verdicts, o, n, set())

    # positive: unique literal of missing 0x10 referenced by paired owner 0x200
    r = run({0x10: {"UNIQUE_LITERAL"}}, {0x200: {"UNIQUE_LITERAL"}})
    checks.append(("unique literal in paired function -> INFOLDED",
                   r[0x10]["class"] == "PRESENT-INFOLDED"
                   and r[0x10]["evidence"]["owner_paired_to"] == 0x20))
    # negative: same literal also used by a second original -> not distinctive
    fo2 = fo + [{"addr": 0x40, "source": "pdata", "tokens": ["d"]}]
    o2 = _FakeSide({0x10: {"UNIQUE_LITERAL"}, 0x40: {"UNIQUE_LITERAL"}}, {}, {}, {}, {}, {})
    n2 = _FakeSide({0x200: {"UNIQUE_LITERAL"}}, {}, {}, {}, {}, {})
    r2 = classify_missing(fo2, pairs, verdicts, o2, n2, set())
    checks.append(("literal shared by two originals is NOT infolded evidence",
                   r2[0x10]["class"] != "PRESENT-INFOLDED"))
    # positive: unique literal referenced only by an UNPAIRED rebuild function
    r3 = run({0x10: {"UNIQUE_LITERAL"}}, {0x999: {"UNIQUE_LITERAL"}})
    checks.append(("literal in unpaired rebuild fn -> PRESENT-UNPAIRED",
                   r3[0x10]["class"] == "PRESENT-UNPAIRED"))
    # negative: literal referenced by the MISSING function's own pair is fine,
    # but owner paired to the SAME original is not an infold
    o4 = _FakeSide({0x10: {"UNIQUE_LITERAL"}}, {}, {}, {}, {}, {})
    n4 = _FakeSide({0x100: {"UNIQUE_LITERAL"}}, {}, {}, {}, {}, {})
    pairs4 = [{"orig_addr": 0x10, "new_addr": 0x100}]
    v4 = {0x10: "RATIO"}   # not MISSING, so skipped entirely
    r4 = classify_missing(fo, pairs4, v4, o4, n4, set())
    checks.append(("a non-MISSING original is never classified",
                   0x10 not in r4))
    # unknown: no anchor at all
    r5 = run({}, {})
    checks.append(("no anchor -> UNKNOWN", r5[0x10]["class"] == "UNKNOWN"))

    passed = sum(1 for _, ok in checks if ok)
    for name, ok in checks:
        print(("PASS " if ok else "FAIL ") + name)
    print(f"self-test: {passed}/{len(checks)} passed")
    return passed == len(checks)


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def source_literals(root):
    """Literals present in our source tree (a HINT for /OPT:REF, not proof)."""
    lits = set()
    if not os.path.isdir(root):
        return lits
    for dirpath, _dirnames, files in os.walk(root):
        for fn in files:
            if not fn.endswith((".cpp", ".h", ".hpp", ".cc", ".js", ".html")):
                continue
            try:
                txt = open(os.path.join(dirpath, fn), "r", errors="ignore").read()
            except OSError:
                continue
            for m in re.finditer(r'"([^"\n]{6,})"', txt):
                lits.add(m.group(1))
    return lits


def summarise(name, recs):
    counts = collections.Counter(r["class"] for r in recs)
    insn = collections.Counter()
    for r in recs:
        insn[r["class"]] += r["orig_insn"]
    lines = [f"{name}: {len(recs)} MISSING real functions, "
             f"{sum(r['orig_insn'] for r in recs)} instructions"]
    for v in ORDER:
        if v not in counts:
            continue
        lines.append(f"  {v:<16} {counts[v]:>4} funcs  {insn[v]:>6} insn")
    return lines, counts, insn


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--orig", default="/home/aviallon/Projects/SexLabPPrism-re/artifacts/SexLabPPrism.dll")
    ap.add_argument("--new", default=None)
    ap.add_argument("--out", default="recon/pairfix-infolding.json")
    ap.add_argument("--report", default="recon/pairfix-infolding.md")
    ap.add_argument("--focus", default=None)
    ap.add_argument("--src", default=os.path.join(HERE, "..", "src"))
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()

    if args.self_test:
        return 0 if self_test() else 1
    if not args.new:
        ap.error("--new is required")

    pe_o, fo, _mo, _ro = M.build_side(args.orig)
    pe_n, fn, _mn, _rn = M.build_side(args.new)
    pairs, _unmA, _unmB, meta = PI.pair_indices(
        fo, fn, args.orig, args.new, per_function="recon/parity-parity/per-function.json")
    M.verdicts(fo, fn, pairs)
    verdicts = {fo[p["i"]]["addr"]: p["verdict"] for p in pairs}
    for f in fo:
        verdicts.setdefault(f["addr"], MISSING)

    # Same Side construction pair_indices uses internally -> identical anchors.
    o = PI.Side(pe_o, fo, N.recover_rtti(args.orig), N.load_recon_names(pe_o))
    n = PI.Side(pe_n, fn, N.recover_rtti(args.new), None)

    src = source_literals(os.path.abspath(args.src))
    recs_map = classify_missing(fo, pairs, verdicts, o, n, src)
    recs = sorted(recs_map.values(), key=lambda r: -r["orig_insn"])

    real = [f for f in fo if f.get("source") == "pdata"]
    before = collections.Counter(verdicts.get(f["addr"], MISSING) for f in real)
    before_insn = collections.Counter()
    for f in real:
        before_insn[verdicts.get(f["addr"], MISSING)] += len(f["tokens"])
    lines, counts, insn = summarise("MISSING classified", recs)

    report = []
    report.append("# PRESENT-INFOLDED classification of the MISSING bucket\n")
    report.append(f"Original: `{args.orig}`  |  Rebuild: `{args.new}`  |  "
                  f"harness: `tools/pairfix_infolding.py`\n")
    report.append("This is a measurement layer on top of `tools/match.py --identity`. "
                  "It changes no pair: PRESENT-INFOLDED is NOT a match and never "
                  "counted as one.\n")
    report.append("## Scoreboard before (real functions, merged .pdata fragments)\n")
    report.append(f"`{len(real)}` real functions, "
                  f"`{sum(len(f['tokens']) for f in real)}` instructions\n")
    report.append("| verdict | funcs | insn |")
    report.append("|---|---|---|")
    for v in ["BYTE-MATCH", "INSN-MATCH", "RATIO", MISSING]:
        if before.get(v):
            report.append(f"| {v} | {before[v]} | {before_insn[v]} |")
    report.append("")
    report.append("## MISSING split (after classification)\n")
    report.extend("    " + l for l in lines)
    report.append("")
    report.append("Total MISSING is preserved: "
                  f"{sum(counts.values())} = "
                  + " + ".join(f"{counts[v]} {v}" for v in ORDER if counts.get(v)))
    report.append("")
    report.append("## Evidence per function\n")
    report.append("| class | orig addr | insn | name | anchor | value | owner | owner paired to |")
    report.append("|---|---|---|---|---|---|---|---|")
    for r in recs:
        ev = r["evidence"] or {}
        val = (ev.get("value") or "")
        if len(val) > 60:
            val = val[:57] + "..."
        on = ev.get("owner_name") or ""
        report.append(
            f"| {r['class']} | {hex(r['orig_addr'])} | {r['orig_insn']} | "
            f"{(r['name'] or '')[:48]} | {ev.get('anchor','')} | "
            f"{val.replace('|','/')} | {hex(ev['owner']) if ev.get('owner') is not None else ''} | "
            f"{hex(ev['owner_paired_to']) if ev.get('owner_paired_to') is not None else ''} | "
            + on[:40] + " |")

    if args.focus:
        fa = int(args.focus, 16)
        for r in recs:
            if r["orig_addr"] == fa:
                report.append(f"\n### FOCUS {args.focus}\n```json\n"
                              + json.dumps(r, indent=2) + "\n```")

    json.dump({"orig": args.orig, "new": args.new,
               "identity_anchors": meta.get("anchors"),
               "real_functions": len(real),
               "pairs": len(pairs),
               "before": dict(before),
               "before_insn": dict(before_insn),
               "missing_classified": recs},
              open(args.out, "w"), indent=1)
    open(args.report, "w").write("\n".join(report) + "\n")

    for l in lines:
        print(l)
    print(f"pairs (unchanged): {len(pairs)}; wrote {args.out}, {args.report}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
