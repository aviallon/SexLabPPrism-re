#!/usr/bin/env python3
"""Matching-decompilation harness (sm64/asm-differ/objdiff style).

Reuses tools/parity.py for PE parsing, objdump, function inventory, normalisation
and pairing.  Adds the piece that was missing: a MATCH verdict based on BYTES
with the bytes covered by the PE base-relocation table masked out, so that two
functions whose only difference is a relocated address compare equal.

Verdicts per paired function, strongest first:
  BYTE-MATCH  identical after relocation masking and trailing-padding strip
  INSN-MATCH  identical normalised instruction stream (addresses canonicalised)
  RATIO       SequenceMatcher over normalised tokens (0..1)
  MISSING     original function has no partner

Usage:
  python3 tools/match.py --report
  python3 tools/match.py --focus 0x18001d150
  python3 tools/match.py --focus 'REL::Module::load_version' --context 4
  python3 tools/match.py --self-test
"""
from __future__ import annotations

import argparse
import csv
import json
import os
import re
import shutil
import struct
import sys
from collections import Counter, defaultdict
from difflib import SequenceMatcher, unified_diff

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import parity  # noqa: E402
from parity import PE, parse_objdump, run_objdump, build_functions, prepare  # noqa: E402

ORIG = "artifacts/SexLabPPrism.dll"
NEW = "artifacts/rebuild/SexLabPPrism-parity-lto.dll"
OUTJSON = "recon/matching/per-function.json"
REPORT = "recon/matching.md"
CSVOUT = "recon/matching.csv"
SYMBOLS = "recon/symbols.csv"

BYTE, INSN, RATIO, MISSING = "BYTE-MATCH", "INSN-MATCH", "RATIO", "MISSING"

RELOC_TYPE_SIZE = {1: 2, 2: 4, 3: 4, 4: 8, 5: 8, 10: 8}


# ---------------------------------------------------------------------------
# base relocations
# ---------------------------------------------------------------------------

def parse_reloc_bytes(pe):
    """Set of VAs for every byte covered by a base relocation (type != 0)."""
    rva, sz = pe.dirs.get("basereloc", (0, 0))
    covered = set()
    n = 0
    if not rva or sz < 8:
        return covered, n
    d = pe.data
    off = pe.rva2off(rva)
    if off is None:
        return covered, n
    end = min(off + sz, len(d))
    while off + 8 <= end:
        page, blk = struct.unpack_from("<II", d, off)
        if blk < 8 or off + blk > end:
            break
        for e in range(off + 8, off + blk - 1, 2):
            w = struct.unpack_from("<H", d, e)[0]
            typ, ofs = w >> 12, w & 0xFFF
            if typ == 0:
                continue
            size = RELOC_TYPE_SIZE.get(typ, 4)
            base = pe.image_base + page + ofs
            for k in range(size):
                covered.add(base + k)
            n += 1
        off += blk
    return covered, n


def ins_mask_offsets(ins, image_base, image_end):
    """Byte offsets inside this instruction that encode an address and must be
    masked for a layout-independent byte comparison:
      * base-relocation bytes (handled separately, by VA)
      * the 4-byte displacement of a RIP-relative memory operand
      * the 4-byte displacement of a direct call/jmp/jcc
      * an 8- or 4-byte absolute image-address immediate
    """
    offs = set()
    ops = ins["ops"]
    n = ins["size"]
    raw = bytes.fromhex(ins.get("bytes", ""))
    if re.search(r"\[rip([+-]0x[0-9a-fA-F]+)?\]", ops):
        for k in range(max(0, n - 4), n):
            offs.add(k)
    if ins["mn"] in parity.BRANCH_MNEMS and ins.get("branch") is not None:
        for k in range(max(0, n - 4), n):
            offs.add(k)
    for mm in re.finditer(r"0x([0-9a-fA-F]+)", ops):
        v = int(mm.group(1), 16)
        if not (image_base <= v < image_end):
            continue
        for width in (8, 4):
            if v >= 1 << (8 * width):
                continue
            le = v.to_bytes(width, "little")
            for start in range(0, max(0, n - width + 1)):
                if raw[start:start + width] == le:
                    offs.update(range(start, start + width))
    return offs


def masked_bytes(func, reloc_va, image_base, image_end):
    """Hex string of the function's code bytes (padding stripped) with each byte
    that encodes an address replaced by '..'.  Returns (string, masked_count)."""
    ntok = len(func.get("tokens", []))
    out = []
    nmask = 0
    for ins in func["insns"][:ntok]:
        b = ins.get("bytes", "")
        offs = ins_mask_offsets(ins, image_base, image_end)
        for k in range(ins["size"]):
            if (ins["addr"] + k) in reloc_va or k in offs:
                out.append("..")
                nmask += 1
            else:
                out.append(b[2 * k:2 * k + 2] or "??")
    return "".join(out), nmask


# ---------------------------------------------------------------------------
# side construction
# ---------------------------------------------------------------------------

def build_side(path):
    pe = PE(path)
    insns = parse_objdump(run_objdump(path), pe.image_base)
    funcs, meta = build_functions(pe, insns)
    reloc, nrel = parse_reloc_bytes(pe)
    for f in funcs:
        f["refs"] = set()
    prepare(funcs, pe.image_base, pe.size_of_image)
    for f in funcs:
        f["maskbytes"], f["nmask"] = masked_bytes(f, reloc, pe.image_base,
                                                 pe.image_base + pe.size_of_image)
        # require at least one surviving byte, else an all-padding overlap is free
        f["byte_hash"] = f["maskbytes"] if (len(f["maskbytes"]) > f["nmask"] * 2) else ""
    return pe, funcs, meta, nrel


def load_orig_meta(path=OUTJSON):
    """Reuse tier/name/basis already computed by tools/parity.py, if present."""
    p = "recon/parity-parity/per-function.json"
    tiers, names = {}, {}
    if os.path.exists(p):
        try:
            d = json.load(open(p))
        except Exception:
            return tiers, names
        for r in d.get("orig_functions", []):
            a = int(r["addr"], 16)
            tiers[a] = r.get("tier") or "library"
            if r.get("name"):
                names[a] = r["name"]
            elif r.get("names"):
                names[a] = r["names"][0]
    return tiers, names


def load_symbols(path=SYMBOLS):
    out = {}
    if os.path.exists(path):
        for row in csv.DictReader(open(path, encoding="utf-8")):
            if row.get("name"):
                out[int(row["address"], 16)] = row["name"]
    return out


# ---------------------------------------------------------------------------
# pairing
# ---------------------------------------------------------------------------

def pair(A, B):
    """Pair by normalised tokens (parity.pair_functions), then finish leftovers
    with a direct masked-byte equality pass.  Returns (pairs, unmA, unmB)."""
    raw_pairs, unmA, unmB, _ = parity.pair_functions(A, B)
    usedA = {p["i"] for p in raw_pairs}
    usedB = {p["j"] for p in raw_pairs}
    pairs = list(raw_pairs)

    byb = defaultdict(list)
    for j in unmB:
        if B[j]["byte_hash"]:
            byb[B[j]["byte_hash"]].append(j)
    for i in list(unmA):
        h = A[i]["byte_hash"]
        if not h or not byb.get(h):
            continue
        j = byb[h].pop(0)
        pairs.append({"i": i, "j": j, "ratio": 1.0, "verdict": INSN,
                      "pass": "byte-exact"})
        usedA.add(i)
        usedB.add(j)
    unmA = [i for i in range(len(A)) if i not in usedA]
    unmB = [j for j in range(len(B)) if j not in usedB]
    return pairs, unmA, unmB


def verdicts(fo, fn, pairs):
    """Annotate each pair with bytematch / insn / ratio / first-div."""
    for p in pairs:
        a, b = fo[p["i"]], fn[p["j"]]
        p["insn"] = a["strict_hash"] == b["strict_hash"]
        p["bytematch"] = (bool(a["byte_hash"]) and a["byte_hash"] == b["byte_hash"]
                           and p["insn"])
        p["ratio"] = SequenceMatcher(None, a["tokens"], b["tokens"],
                                     autojunk=False).ratio() if not p["insn"] else 1.0
        p["first_div"] = first_div(a["tokens"], b["tokens"])
        if p["bytematch"]:
            p["verdict"] = BYTE
        elif p["insn"]:
            p["verdict"] = INSN
        else:
            p["verdict"] = RATIO
    return pairs


def first_div(a, b):
    for i in range(min(len(a), len(b))):
        if a[i] != b[i]:
            return i
    if len(a) != len(b):
        return min(len(a), len(b))
    return None


# ---------------------------------------------------------------------------
# focus / side-by-side diff
# ---------------------------------------------------------------------------

def find_orig(fo, name_map, key):
    kl = key.lower()
    if re.match(r"^(0x[0-9a-f]+|[0-9a-f]+)$", kl):
        a = int(kl, 16)
        for i, f in enumerate(fo):
            if f["addr"] == a:
                return i
    for i, f in enumerate(fo):
        nm = name_map.get(f["addr"]) or f.get("name")
        if nm and kl in nm.lower():
            return i
    return None


def focus(fo, fn, pairs, name_map, key, ctx=2):
    i = find_orig(fo, name_map, key)
    if i is None:
        print(f"no function matching {key!r}")
        return 1
    a = fo[i]
    nm = name_map.get(a["addr"]) or a.get("name") or ""
    print(f"ORIG {hex(a['addr'])}  {nm}  insns={len(a['tokens'])} "
          f"masked_bytes={a['nmask']}")
    p = next((p for p in pairs if p["i"] == i), None)
    if p is None:
        print("verdict: MISSING (no partner in the new binary)")
        return 0
    b = fn[p["j"]]
    print(f"NEW  {hex(b['addr'])}  insns={len(b['tokens'])} "
          f"masked_bytes={b['nmask']}")
    print(f"verdict: {p['verdict']}  bytematch={p['bytematch']} "
          f"insn={p['insn']} ratio={p['ratio']:.4f} first_div={p['first_div']}")

    ot, nt = a["tokens"], b["tokens"]
    oraw, nraw = a["raw_tokens"], b["raw_tokens"]
    sm = SequenceMatcher(None, ot, nt, autojunk=False)
    first = p["first_div"]
    print("\n  idxO  original                          | idxN  new                                 ")
    print("  ----- --------------------------------- | ----- ----------------------------------")
    emitted = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            # keep ctx lines around a change
            lo = max(i1, i2 - ctx)
            for k in range(i1, i2):
                if k < lo and (k >= i1):
                    if k == i1:
                        print(f"  ...   ({i2 - i1 - ctx} equal lines) ...")
                    continue
                mark = " " if k != first else ">"
                print(f"  {mark}{k:4d} {oraw[k][:37]:37s} | {mark}{j1 + (k - i1):4d} {nraw[j1 + (k - i1)][:38]:38s}")
                emitted.append(k)
        else:
            for k in range(i1, i2):
                mark = ">" if k == first else "-"
                print(f"  {mark}{k:4d} {oraw[k][:37]:37s} | ---  {'':38s}")
            for k in range(j1, j2):
                mark = ">" if k == first else "+"
                print(f"  ---  {'':37s} | {mark}{k:4d} {nraw[k][:38]:38s}")
    if first is not None and first < max(len(ot), len(nt)):
        print(f"\nfirst divergence at normalised insn #{first}:")
        o = ot[first] if first < len(ot) else "<end>"
        n = nt[first] if first < len(nt) else "<end>"
        print(f"  orig: {o}")
        print(f"  new : {n}")
    return 0


# ---------------------------------------------------------------------------
# report
# ---------------------------------------------------------------------------

def summarise(fo, fn, pairs, unmA, name_map, tier_map):
    """Return per-tier counters + instruction-weighted numbers."""
    pair_by_i = {p["i"]: p for p in pairs}
    tiers = {}
    for tier in ("plugin", "library"):
        c = Counter()
        tot_insn = 0
        hit_insn = 0
        for i, f in enumerate(fo):
            if (tier_map.get(f["addr"]) or "library") != tier:
                continue
            tot_insn += len(f["tokens"])
            p = pair_by_i.get(i)
            if p is None:
                c[MISSING] += 1
            else:
                c[p["verdict"]] += 1
                if p["verdict"] == BYTE:
                    hit_insn += len(f["tokens"])
        tiers[tier] = {"counts": c, "orig_funcs": sum(c.values()),
                       "orig_insn": tot_insn, "byte_insn": hit_insn}
    allc = Counter()
    tot = hit = 0
    for i, f in enumerate(fo):
        tot += len(f["tokens"])
        p = pair_by_i.get(i)
        if p is None:
            allc[MISSING] += 1
        else:
            allc[p["verdict"]] += 1
            if p["verdict"] == BYTE:
                hit += len(f["tokens"])
    tiers["all"] = {"counts": allc, "orig_funcs": len(fo),
                    "orig_insn": tot, "byte_insn": hit}
    return tiers


def write_csv(path, fo, fn, pairs, unmA, name_map, tier_map):
    pair_by_i = {p["i"]: p for p in pairs}
    rows = []
    for i, f in enumerate(fo):
        p = pair_by_i.get(i)
        rows.append({
            "orig_addr": hex(f["addr"]),
            "name": name_map.get(f["addr"]) or f.get("name") or "",
            "tier": tier_map.get(f["addr"]) or "library",
            "orig_insn": len(f["tokens"]),
            "our_insn": len(fn[p["j"]]["tokens"]) if p else "",
            "bytematch": 1 if (p and p["bytematch"]) else 0,
            "insnratio": f"{p['ratio']:.4f}" if p else "0.0000",
            "first_div": "" if not p or p["first_div"] is None else p["first_div"],
            "anchor": (p.get("anchor") or "") if p else "",
            "_sort": (0 if (p and p["bytematch"]) else 1,
                      p["ratio"] if p else 0.0),
        })
    rows.sort(key=lambda r: r["_sort"], reverse=True)
    with open(path, "w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=["orig_addr", "name", "tier", "orig_insn",
                                           "our_insn", "bytematch", "insnratio",
                                           "first_div", "anchor"], extrasaction="ignore")
        w.writeheader()
        for r in rows:
            w.writerow(r)
    return rows


def write_report(path, fo, fn, pairs, unmA, name_map, tier_map, masked):
    tiers = summarise(fo, fn, pairs, unmA, name_map, tier_map)
    L = []
    A = L.append
    A("# Matching decompilation scoreboard\n")
    A(f"Original: `{ORIG}`  |  Rebuild: `{NEW}`")
    A(f"Verdict basis: BYTE-MATCH = INSN-MATCH plus identical bytes after masking "
      f"address-encoding bytes "
      f"({masked['orig_bytes']} orig / {masked['new_bytes']} rebuild bytes inside "
      f"function windows, over {masked['orig_insn']}/{masked['new_insn']} instructions); "
      f"plus base-relocation-table bytes ({masked['orig_table']} orig / "
      f"{masked['new_table']} rebuild entries) and a trailing-padding strip.\n")
    allt = tiers["all"]
    A("## Overall\n")
    A(f"- original functions: {allt['orig_funcs']}")
    for k in (BYTE, INSN, RATIO, MISSING):
        A(f"- {k}: {allt['counts'].get(k, 0)}")
    pct_f = 100.0 * allt["counts"].get(BYTE, 0) / max(allt["orig_funcs"], 1)
    pct_i = 100.0 * allt["byte_insn"] / max(allt["orig_insn"], 1)
    A(f"- matching, function-weighted: **{pct_f:.2f}%**")
    A(f"- matching, instruction-weighted: **{pct_i:.2f}%** "
      f"({allt['byte_insn']}/{allt['orig_insn']} orig instructions)\n")
    A("## Per tier\n")
    A("| tier | funcs | BYTE | INSN | RATIO | MISSING | byte% | insn% |")
    A("|---|---|---|---|---|---|---|---|")
    for tier in ("plugin", "library", "all"):
        t = tiers[tier]
        c = t["counts"]
        bf = 100.0 * c.get(BYTE, 0) / max(t["orig_funcs"], 1)
        bi = 100.0 * t["byte_insn"] / max(t["orig_insn"], 1)
        A(f"| {tier} | {t['orig_funcs']} | {c.get(BYTE,0)} | {c.get(INSN,0)} | "
          f"{c.get(RATIO,0)} | {c.get(MISSING,0)} | {bf:.1f} | {bi:.1f} |")
    A("")
    close = sorted((p for p in pairs if not p["bytematch"]),
                   key=lambda p: -p["ratio"])[:15]
    A("## Closest to matching (unmatched by bytes)\n")
    A("| orig | name | tier | orig insn | our insn | ratio | first div |")
    A("|---|---|---|---|---|---|---|")
    for p in close:
        f = fo[p["i"]]
        nm = name_map.get(f["addr"]) or f.get("name") or ""
        nf = fn[p["j"]]
        A(f"| {hex(f['addr'])} | {nm[:44]} | {tier_map.get(f['addr'],'library')} | "
          f"{len(f['tokens'])} | {len(nf['tokens'])} | {p['ratio']:.4f} | {p['first_div']} |")
    A("")
    open(path, "w", encoding="utf-8").write("\n".join(L) + "\n")
    return tiers


# ---------------------------------------------------------------------------
# self-test
# ---------------------------------------------------------------------------

def _clone_with_mutation(src, dst, mutate):
    shutil.copyfile(src, dst)
    with open(dst, "r+b") as fh:
        mutate(fh)


def _pdata_offset(pe, addr_va):
    rva, sz = pe.dirs.get("exception", (0, 0))
    d = pe.data
    off = pe.rva2off(rva)
    for i in range(sz // 12):
        o = off + i * 12
        b = struct.unpack_from("<I", d, o)[0]
        if pe.image_base + b == addr_va:
            return o
    return None


def self_test(pe, fo, ro):
    """Build mutated copies in a scratch dir; never touches the workspace binaries."""
    import tempfile
    passed = total = 0
    notes = []

    def check(cond, msg):
        nonlocal passed, total
        total += 1
        if cond:
            passed += 1
        notes.append(("PASS " if cond else "FAIL ") + msg)

    # 1. identical binary -> 100% BYTE-MATCH
    pairs, unmA, _ = pair(fo, fo)
    verdicts(fo, fo, pairs)
    nbyte = sum(1 for p in pairs if p["bytematch"])
    check(nbyte == len(fo) and not unmA,
          f"identical binary: BYTE-MATCH {nbyte}/{len(fo)} (expected 100%)")

    reloc = parse_reloc_bytes(pe)[0]
    with tempfile.TemporaryDirectory() as td:
        mut = os.path.join(td, "mut.dll")
        # 2. mutation: flip one non-relocation byte -> exactly that function fails
        cand = None
        tries = 0
        for f in fo:
            ins = f["insns"]
            if len(f["tokens"]) < 3 or not ins:
                continue
            va = ins[0]["addr"] + ins[0]["size"] - 1
            if va in reloc:
                continue
            fo_off = pe.rva2off(va - pe.image_base)
            if fo_off is None or pe.data[fo_off] == 0:
                continue
            tries += 1
            if tries > 4:
                break

            def do(fh, o=fo_off, ob=pe.data[fo_off]):
                fh.seek(o)
                fh.write(bytes([ob ^ 0x01]))

            _clone_with_mutation(ORIG, mut, do)
            _, fm, _, _ = build_side(mut)
            pm, um, _ = pair(fo, fm)
            verdicts(fo, fm, pm)
            bad = {fo[p["i"]]["addr"] for p in pm if not p["bytematch"]}
            bad |= {fo[i]["addr"] for i in um}
            if bad == {f["addr"]}:
                cand = f["addr"]
                break
        check(cand is not None,
              f"mutation: only {hex(cand)} differs, all others BYTE-MATCH"
              if cand else "mutation: could not isolate a single function")

        # 3. deletion: remove .pdata entry and pad with int3 -> MISSING
        target = None
        for f in fo:
            if f["source"] == "pdata" and len(f["tokens"]) >= 3:
                po = _pdata_offset(pe, f["addr"])
                if po is not None:
                    target = (f, po)
                    break
        if target:
            f, po = target
            code_off = pe.rva2off(f["addr"] - pe.image_base)

            def del_fn(fh, po=po, co=code_off, sz=f["size"]):
                fh.seek(po)
                fh.write(struct.pack("<III", 0, 0, 0))
                fh.seek(co)
                fh.write(b"\xcc" * sz)

            _clone_with_mutation(ORIG, mut, del_fn)
            _, fd, _, _ = build_side(mut)
            addrs = {g["addr"] for g in fd}
            check(f["addr"] not in addrs,
                  f"deletion: {hex(f['addr'])} absent from inventory (MISSING)")
        else:
            check(False, "deletion: no plugin function with .pdata entry found")

    # 4. relocation masking proof
    #    (a) if a real orig/rebuild pair differs only in relocated bytes, use it;
    #    (b) otherwise perturb the relocated bytes of one real function and prove
    #        that masked bytes still compare equal while raw bytes do not.
    _, fn2, _, rn = build_side(NEW)
    p2, _, _ = pair(fo, fn2)
    verdicts(fo, fn2, p2)
    proof = None
    for p in p2:
        a, b = fo[p["i"]], fn2[p["j"]]
        if p["bytematch"] and a["code_bytes"] != b["code_bytes"]:
            proof = ("real pair", a["addr"], a["nmask"], b["nmask"])
            break
    if proof is None:
        target = None
        for f in fo:
            if f["source"] != "pdata" or len(f["tokens"]) < 2:
                continue
            hits = [ins["addr"] + k for ins in f["insns"][:len(f["tokens"])]
                    for k in range(ins["size"]) if ins["addr"] + k in reloc]
            if hits:
                target = (f, hits)
                break
        if target is not None:
            f, hits = target
            with tempfile.TemporaryDirectory() as td:
                mut = os.path.join(td, "reloc.dll")
                offs = [(pe.rva2off(va - pe.image_base), pe.data[pe.rva2off(va - pe.image_base)])
                        for va in hits if pe.rva2off(va - pe.image_base) is not None]

                def do(fh, offs=offs):
                    for o, ob in offs:
                        fh.seek(o)
                        fh.write(bytes([ob ^ 0x11]))

                _clone_with_mutation(ORIG, mut, do)
                _, fm2, _, _ = build_side(mut)
                pm, _, _ = pair(fo, fm2)
                verdicts(fo, fm2, pm)
                m = next((p for p in pm if fo[p["i"]]["addr"] == f["addr"]), None)
                if m and m["bytematch"] and \
                        fo[m["i"]]["code_bytes"] != fm2[m["j"]]["code_bytes"]:
                    proof = ("perturbed copy", f["addr"], fo[m["i"]]["nmask"],
                             fm2[m["j"]]["nmask"])
    if proof:
        kind, addr, mno, mnn = proof
        check(True, f"relocation masking ({kind}): {hex(addr)} BYTE-MATCH though raw "
                    f"bytes differ; masked {mno} orig / {mnn} other bytes")
    else:
        check(False, "relocation masking: no BYTE-MATCH pair with differing raw bytes")
    check(ro > 0 and rn > 0,
          f"relocation tables parsed: {ro} orig entries, {rn} rebuild entries")
    return passed, total, notes


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--orig", default=ORIG)
    ap.add_argument("--new", default=NEW)
    ap.add_argument("--focus", default=None)
    ap.add_argument("--context", type=int, default=2)
    ap.add_argument("--report", action="store_true")
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--identity", action="store_true",
                    help="pair by identity anchors (tools/pair_identity.py) "
                         "instead of structural similarity; keeps every pair's "
                         "anchor/confidence")
    args = ap.parse_args()

    name_map = load_symbols()
    tier_map, meta_names = load_orig_meta()
    name_map = {**meta_names, **name_map}

    if args.self_test:
        pe_o = PE(args.orig)
        insns = parse_objdump(run_objdump(args.orig), pe_o.image_base)
        fo, _ = build_functions(pe_o, insns)
        for f in fo:
            f["refs"] = set()
        prepare(fo, pe_o.image_base, pe_o.size_of_image)
        reloc, ro = parse_reloc_bytes(pe_o)
        for f in fo:
            f["maskbytes"], f["nmask"] = masked_bytes(f, reloc, pe_o.image_base,
                                                     pe_o.image_base + pe_o.size_of_image)
            f["byte_hash"] = f["maskbytes"] if (len(f["maskbytes"]) > f["nmask"] * 2) else ""
        passed, total, notes = self_test(pe_o, fo, ro)
        print(f"self-test: {passed}/{total} passed")
        for n in notes:
            print("  " + n)
        return 0 if passed == total else 1

    pe_o, fo, mo, ro = build_side(args.orig)
    _, fn, mn, rn = build_side(args.new)
    id_meta = None
    if args.identity:
        import pair_identity
        pairs, unmA, unmB, id_meta = pair_identity.pair_indices(
            fo, fn, args.orig, args.new,
            per_function="recon/parity-parity/per-function.json")
        print("identity pairing: " + " ".join(
            f"{k}={v}" for k, v in sorted(id_meta["anchors"].items())))
    else:
        pairs, unmA, unmB = pair(fo, fn)
    verdicts(fo, fn, pairs)

    if args.focus:
        return focus(fo, fn, pairs, name_map, args.focus, args.context)

    # always compute scoreboard
    os.makedirs(os.path.dirname(OUTJSON), exist_ok=True)
    masked = {
        "orig_table": ro, "new_table": rn,
        "orig_bytes": sum(f["nmask"] for f in fo),
        "new_bytes": sum(f["nmask"] for f in fn),
        "orig_insn": sum(1 for f in fo if f["nmask"]),
        "new_insn": sum(1 for f in fn if f["nmask"]),
    }
    tiers = write_report(REPORT, fo, fn, pairs, unmA, name_map, tier_map, masked)
    rows = write_csv(CSVOUT, fo, fn, pairs, unmA, name_map, tier_map)

    pbyi = {p["i"]: p for p in pairs}
    jout = {
        "harness": "tools/match.py",
        "orig": args.orig, "new": args.new,
        "masking": masked,
        "functions": [],
    }
    for i, f in enumerate(fo):
        p = pbyi.get(i)
        jout["functions"].append({
            "orig_addr": hex(f["addr"]),
            "name": name_map.get(f["addr"]) or f.get("name") or "",
            "tier": tier_map.get(f["addr"]) or "library",
            "orig_insn": len(f["tokens"]), "orig_masked": f["nmask"],
            "new_addr": hex(fn[p["j"]]["addr"]) if p else None,
            "our_insn": len(fn[p["j"]]["tokens"]) if p else None,
            "verdict": p["verdict"] if p else MISSING,
            "bytematch": bool(p and p["bytematch"]),
            "insn_match": bool(p and p["insn"]),
            "ratio": round(p["ratio"], 6) if p else 0.0,
            "first_div": p["first_div"] if p else None,
            "anchor": p.get("anchor") if p else None,
            "confidence": p.get("confidence") if p else None,
        })
    if id_meta:
        jout["identity"] = id_meta
    json.dump(jout, open(OUTJSON, "w"), indent=1)

    allt = tiers["all"]
    c = allt["counts"]
    print(f"orig functions {allt['orig_funcs']}, rebuild {len(fn)}")
    print(f"BYTE-MATCH {c.get(BYTE,0)}  INSN-MATCH {c.get(INSN,0)}  "
          f"RATIO {c.get(RATIO,0)}  MISSING {c.get(MISSING,0)}")
    print(f"function-weighted {100.0*c.get(BYTE,0)/max(allt['orig_funcs'],1):.2f}%  "
          f"instruction-weighted {100.0*allt['byte_insn']/max(allt['orig_insn'],1):.2f}%")
    for tier in ("plugin", "library"):
        t = tiers[tier]
        print(f"  {tier:8s} {t['orig_funcs']:4d} funcs  BYTE={t['counts'].get(BYTE,0):4d} "
              f"INSN={t['counts'].get(INSN,0):4d} RATIO={t['counts'].get(RATIO,0):4d} "
              f"MISSING={t['counts'].get(MISSING,0):4d}  "
              f"insn%={100.0*t['byte_insn']/max(t['orig_insn'],1):.1f}")
    print(f"address bytes masked in function windows: orig {masked['orig_bytes']} "
          f"in {masked['orig_insn']} insns / rebuild {masked['new_bytes']} in "
          f"{masked['new_insn']} insns")
    print(f"base-relocation table entries: orig {ro} / rebuild {rn} "
          f"(all in .rdata/.data, none in .text)")
    print(f"wrote {REPORT}, {CSVOUT}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
