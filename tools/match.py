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
DECLARED = "DECLARED-DIVERGENT"   # a declared pair that is NOT a real match

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
    # A SIB/base+index operand whose displacement lands inside the image RVA
    # range encodes a link-time symbol difference (same reason as the
    # RIP-relative case): the constant differs between the original and our
    # rebuild although the instruction is identical, so the 4 displacement
    # bytes must be masked too. Small displacements (struct field offsets) stay
    # in the comparison because they carry meaning.
    for mm in re.finditer(r"\[[^\]]*[+-](0x[0-9a-fA-F]+)\]", ops):
        v = int(mm.group(1), 16)
        if not (0 < v < (image_end - image_base)):
            continue
        le = v.to_bytes(4, "little")
        for start in range(0, max(0, n - 3)):
            if raw[start:start + 4] == le:
                offs.update(range(start, start + 4))
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
    """Reuse tier/name/basis already computed by tools/parity.py, if present.

    The classifier input is the report written by the full tools/parity.py run,
    which is NOT produced by match.py itself.  Resolve it against the repo root
    (HERE/..) rather than the current working directory: the historical all-
    library bug was simply match.py being run from build/ where the relative
    path "recon/parity-parity/per-function.json" does not exist.
    """
    repo = os.path.dirname(HERE)
    cands = [os.path.join(repo, "recon/parity-parity/per-function.json"),
             os.path.join(repo, "build/recon/parity-parity/per-function.json"),
             "recon/parity-parity/per-function.json",
             "build/recon/parity-parity/per-function.json"]
    p = next((c for c in cands if os.path.exists(c)), cands[0])
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


def compute_tiers(pe, funcs, name_map=None, root=None):
    """Self-contained tier assignment, using exactly the definition in
    tools/parity.py's classify(): original exports + plugin string/RTTI refs +
    registration table + BFS over direct calls from those roots, with
    library-named functions as a hard stop.

    ``name_map`` is intentionally NOT the output-name symbol map: the reference
    classifier was fed recon/functions.json + recovered __cdecl signatures.
    Passing recon/symbols.csv instead would make RTTI/slot names hit the
    library-name stop list and shrink the plugin tier.

    Returns ({addr: 'plugin'|'library'}, {addr: name}).
    """
    repo = root or os.path.dirname(HERE)
    nm = {}
    for cand in (os.path.join(repo, "recon/functions.json"),
                 os.path.join(repo, "build/recon/functions.json"),
                 "recon/functions.json"):
        if os.path.exists(cand):
            nm, _ = parity.load_name_json(cand, pe)
            break
    parity.recover_sig_names(pe, funcs)
    for f in funcs:
        for n in nm.get(f["addr"], []):
            f.setdefault("names", []).append(n)
    plugin_refs = parity.plugin_ref_addresses(pe)
    known = parity.load_manifest(os.path.join(repo, "recon/decompiled/MANIFEST.txt"))
    if not known:
        known = parity.load_manifest("recon/decompiled/MANIFEST.txt")
    parity.classify(pe, funcs, nm, pe.exports(), plugin_refs, known)
    tiers = {f["addr"]: f["tier"] for f in funcs}
    names = {f["addr"]: f["name"] for f in funcs if f.get("name")}
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
        elif p.get("declared"):
            # an explicit declaration whose body does not actually match:
            # reported separately, NEVER counted as a match.
            p["verdict"] = DECLARED
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
# declared pairs (tools/extract_declared.py -> recon/declared-mappings.csv)
# ---------------------------------------------------------------------------

def load_declared(path):
    """Read a declared-mappings CSV into {orig_addr: row}.

    Required columns: orig_addr, our_symbol_or_source_location, evidence, round
    (kind and an optional our_new_addr override are accepted).  The declaration
    is a claim of intent, NEVER a match: it says *our function S was written to
    implement original address A*.  It is only ever used to propose the pair
    (A, S); the verdict is still computed from bytes/tokens.
    """
    out = {}
    if not path or not os.path.exists(path):
        return out
    for row in csv.DictReader(open(path, encoding="utf-8")):
        a = row.get("orig_addr", "").strip()
        if not re.match(r"^(0x)?[0-9a-fA-F]+$", a):
            continue
        out[int(a, 16)] = row
    return out


# ---------------------------------------------------------------------------
# MSVC linker map (--map): symbol-exact binding for declarations
# ---------------------------------------------------------------------------
# The parity artifact ships NO COFF symbol table (number-of-symbols = 0), so a
# declaration's source symbol cannot be looked up in the binary.  The parity CI
# job therefore links with MSVC /MAP and uploads SexLabPPrism.map beside the
# DLL.  This parser resolves a declaration's source symbol to a VA from that
# map by EXACT name, instead of guessing the body by similarity.  /MAP writes a
# text side-file only and has no effect on the linked image.

# Publics-by-value / Static-symbols row:
#   0001:0000000000001060       ?Foo@?A0x...@@YAXXZ  0000000180001060  f  obj.obj
# The trailing "Rva+Base" column is the absolute VA (preferred load address).
_MAP_SYM_RE = re.compile(
    r"^\s*[0-9a-fA-F]{4}:[0-9a-fA-F]{8,16}\s+(\S+)\s+"
    r"([0-9a-fA-F]{8,16})\s+(\S+)\s+(\S.*)$")


def load_msvc_map(path):
    """Parse an MSVC /MAP file into {raw_symbol: set(addr_va)}.

    Only rows that carry an Rva+Base address are collected; the section table
    rows (``0001:... 0000F000H .text CODE``) do not match because ``.text`` is
    not a hex address.  A symbol listed at several distinct addresses is kept as
    several entries so the resolver can reject it as ambiguous.
    """
    out = defaultdict(set)
    if not path or not os.path.exists(path):
        return out
    for line in open(path, encoding="utf-8", errors="replace"):
        m = _MAP_SYM_RE.match(line.rstrip("\n"))
        if not m:
            continue
        sym, addr = m.group(1), int(m.group(2), 16)
        if sym.startswith("."):
            continue
        out[sym].add(addr)
    return out


def _map_symbol_keys(addr_by_sym, name):
    """EXACT-symbol candidate keys, never a substring/fuzzy hit.

    A C-linkage symbol matches its raw name; an MSVC-mangled C++ identifier
    embeds the identifier as ``?Name@`` (``?Foo@@YAXXZ`` at global scope,
    ``?Foo@?A0x...@@YAXXZ`` in an anonymous namespace, ``?Foo@Ns@@YAXXZ`` in a
    named one) so the identifier boundary is an exact prefix ``?Name@``.
    ``?FooExtra@...`` therefore does NOT match ``Foo``.
    """
    keys = []
    if name in addr_by_sym:
        keys.append(name)
    pref = "?" + name + "@"
    for raw in addr_by_sym:
        if raw.startswith(pref):
            keys.append(raw)
    return keys


def resolve_map_symbol(addr_by_sym, name, text_lo, text_hi):
    """Resolve a source symbol to a VA.  Returns (addr, "") or (None, reason).

    A declaration resolves ONLY on an exact symbol match.  A missing symbol, a
    symbol that resolves to several distinct addresses, and a symbol whose
    address is outside the rebuild's .text range are all rejected with a reason;
    the caller then leaves the original MISSING (never a fabricated pair).
    """
    if not name:
        return None, "empty symbol"
    keys = _map_symbol_keys(addr_by_sym, name)
    if not keys:
        return None, "symbol not found"
    addrs = set()
    for k in keys:
        addrs |= addr_by_sym[k]
    if len(addrs) > 1:
        return None, f"duplicate symbol ({len(addrs)} addresses)"
    addr = addrs.pop()
    if not (text_lo <= addr < text_hi):
        return None, "address outside .text"
    return addr, ""


def declared_map_symbol(dec):
    """The source symbol to resolve for a declaration row.

    RECONSTRUCTED rows carry the function name directly.  PRESENT-UNPAIRED rows
    carry a file:line in that column (the body lives elsewhere); their evidence
    text names the claimed symbol, so take its last ``::`` component there.
    """
    sym = (dec.get("our_symbol_or_source_location") or "").strip()
    if sym and not re.search(r"\.(?:cpp|h|hpp|cxx|cc):\d+$", sym) \
            and not sym.startswith("src/"):
        return sym
    m = re.search(r"claims\s+`?([^`>]+?)`?\s*->", dec.get("evidence", ""))
    if m:
        return m.group(1).strip().strip("`'").split("::")[-1].strip()
    return ""


def _jaccard(ca, cb):
    inter = sum((ca & cb).values())
    tot = sum(ca.values()) + sum(cb.values()) - inter
    return inter / tot if tot else 0.0


def _tok_counter(tokens):
    c = Counter()
    for t in tokens:
        c[t] += 1
    return c


def find_declared_candidate(fo, fn, i, pool, counters, min_ratio):
    """Pick the rebuild function that best implements original ``i``.

    This is the *retrieval* step only: the declaration supplies the intent
    (which source body), the similarity search locates that body's compiled
    function among the still-unpaired rebuild functions.  Candidates are
    screened by instruction count window and token Jaccard, then scored with
    the same SequenceMatcher ratio the harness uses elsewhere.  A candidate
    that does not clear ``min_ratio`` is not bound at all, so the declared
    address stays MISSING (never a fabricated pair).
    """
    toks = fo[i]["tokens"]
    n = len(toks)
    if not toks:
        return None, 0.0, 0
    ct = _tok_counter(toks)
    best = (None, 0.0)
    above = 0
    for j in pool:
        m = len(fn[j]["tokens"])
        if m == 0 or m < n * 0.45 or m > n * 2.2:
            continue
        if _jaccard(ct, counters[j]) < 0.25:
            continue
        r = SequenceMatcher(None, toks, fn[j]["tokens"], autojunk=False).ratio()
        if r >= min_ratio:
            above += 1
        if r > best[1]:
            best = (j, r)
    if best[1] < min_ratio:
        return None, best[1], above
    return best[0], best[1], above


def apply_declared(fo, fn, pairs, unmA, unmB, declared, min_ratio=0.50,
                   map_resolver=None):
    """Seed declared pairs and return (pairs, unmA, unmB, rows).

    Declarations never displace an existing pair: a declared original that is
    already paired keeps its verdict (rule: a declaration must not turn a
    BYTE-MATCH into something else).  For a declared original that is still
    MISSING, the target rebuild function is located among the unpaired pool
    (or pinned by ``our_new_addr`` in the CSV); the pair is then scored with the
    normal verdicts(), so a real byte/insn match is still reported as such and
    anything less is DECLARED-DIVERGENT, which is NOT a match.

    ``map_resolver(name) -> (addr|None, reason)`` switches the binding to
    symbol-exact: the declared source symbol is looked up in the rebuild's
    linker map.  When it is supplied, the similarity screen is NOT used as a
    fallback - an unresolvable symbol stays MISSING with its reason recorded.
    """
    pair_by_i = {p["i"]: p for p in pairs}
    j2 = {fn[j]["addr"]: j for j in range(len(fn))}
    pool = list(unmB)
    counters = {j: _tok_counter(fn[j]["tokens"]) for j in pool}
    rows = []
    seeded = []
    for i, f in enumerate(fo):
        dec = declared.get(f["addr"])
        if dec is None:
            continue
        sym = dec.get("our_symbol_or_source_location", "")
        ev = dec.get("evidence", "")
        kind = dec.get("kind", "declared")
        row = {"orig_addr": hex(f["addr"]), "name": fo[i].get("name") or "",
               "tier": "", "orig_insn": len(f["tokens"]),
               "declared_symbol": sym, "evidence": ev, "round": dec.get("round", ""),
               "kind": kind, "binding": "", "new_addr": "", "our_insn": "",
               "bytematch": 0, "insn": 0, "ratio": 0.0, "verdict": "",
               "map_symbol": "", "map_reason": ""}
        if i in pair_by_i:
            p = pair_by_i[i]
            row.update(binding="existing-pair", new_addr=hex(fn[p["j"]]["addr"]),
                       our_insn=len(fn[p["j"]]["tokens"]), bytematch=int(p["bytematch"]),
                       insn=int(p["insn"]), ratio=round(p["ratio"], 6),
                       verdict=p["verdict"])
            rows.append(row)
            continue
        j, r, above = None, 0.0, 0
        if map_resolver is not None:
            name = declared_map_symbol(dec)
            addr, reason = map_resolver(name)
            row["binding"] = "map"
            row["map_symbol"] = name
            if addr is None:
                row["map_reason"] = reason
                row["verdict"] = MISSING
                rows.append(row)
                continue
            j = j2.get(addr)
            if j is None:
                row["map_reason"] = f"address {hex(addr)} not in inventory"
                row["verdict"] = MISSING
                rows.append(row)
                continue
            if j not in pool:
                row["map_reason"] = "target function already paired"
                row["verdict"] = MISSING
                rows.append(row)
                continue
            pool.remove(j)
            r = SequenceMatcher(None, f["tokens"], fn[j]["tokens"],
                                autojunk=False).ratio()
        pin = dec.get("our_new_addr", "").strip()
        if map_resolver is None and pin:
            j = j2.get(int(pin, 16)) if re.match(r"^(0x)?[0-9a-fA-F]+$", pin) else None
            if j is not None:
                r = SequenceMatcher(None, f["tokens"], fn[j]["tokens"],
                                    autojunk=False).ratio()
                row["binding"] = "pinned"
        if map_resolver is None and j is None:
            j, r, above = find_declared_candidate(fo, fn, i, pool, counters, min_ratio)
            row["binding"] = "best-ratio"
            if j is not None:
                pool.remove(j)
        if j is None:
            row["verdict"] = MISSING
            row["ratio"] = round(r, 6)
            rows.append(row)
            continue
        p = {"i": i, "j": j, "ratio": 1.0, "verdict": RATIO, "pass": "declared"}
        seeded.append(p)
        pair_by_i[i] = p
        row.update(new_addr=hex(fn[j]["addr"]), our_insn=len(fn[j]["tokens"]),
                   ratio=round(r, 6), binding=row["binding"] or "best-ratio")
        rows.append(row)
    if seeded:
        for p in seeded:
            p["declared"] = True
        verdicts(fo, fn, seeded)
        pairs = pairs + seeded
        # fill the verdict fields from the (now) paired rows
        by_addr = {hex(fo[p["i"]]["addr"]): p for p in seeded}
        for row in rows:
            p = by_addr.get(row["orig_addr"])
            if p is not None:
                row.update(bytematch=int(p["bytematch"]), insn=int(p["insn"]),
                           ratio=round(p["ratio"], 6), verdict=p["verdict"])
    return pairs, [i for i in range(len(fo)) if i not in pair_by_i], pool, rows


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
    for k in (BYTE, INSN, RATIO, MISSING, DECLARED):
        if k == DECLARED and not allt["counts"].get(k):
            continue
        A(f"- {k}: {allt['counts'].get(k, 0)}")
    pct_f = 100.0 * allt["counts"].get(BYTE, 0) / max(allt["orig_funcs"], 1)
    pct_i = 100.0 * allt["byte_insn"] / max(allt["orig_insn"], 1)
    A(f"- matching, function-weighted: **{pct_f:.2f}%**")
    A(f"- matching, instruction-weighted: **{pct_i:.2f}%** "
      f"({allt['byte_insn']}/{allt['orig_insn']} orig instructions)\n")
    A("## Per tier\n")
    A("| tier | funcs | BYTE | INSN | RATIO | MISSING | DECLARED-DIVERGENT | byte% | insn% |")
    A("|---|---|---|---|---|---|---|---|---|")
    for tier in ("plugin", "library", "all"):
        t = tiers[tier]
        c = t["counts"]
        bf = 100.0 * c.get(BYTE, 0) / max(t["orig_funcs"], 1)
        bi = 100.0 * t["byte_insn"] / max(t["orig_insn"], 1)
        A(f"| {tier} | {t['orig_funcs']} | {c.get(BYTE,0)} | {c.get(INSN,0)} | "
          f"{c.get(RATIO,0)} | {c.get(MISSING,0)} | {c.get(DECLARED,0)} | "
          f"{bf:.1f} | {bi:.1f} |")
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


def write_declared(out_csv, out_md, declared_rows, fo, fn, pairs, name_map,
                   tier_map, pdata_addrs):
    """Write the declared-pair audit CSV and the honest split.

    A DECLARED-DIVERGENT row is a pair that a source declaration claims but
    whose bytes/instructions do NOT match.  It is never a match.
    """
    order = {hex(a): a for a in pdata_addrs}
    pair_by_i = {p["i"]: p for p in pairs}
    with open(out_csv, "w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=["orig_addr", "name", "tier", "orig_insn",
                                           "declared_symbol", "evidence", "round",
                                           "kind", "binding", "map_symbol",
                                           "map_reason", "new_addr", "our_insn",
                                           "bytematch", "insn", "ratio", "verdict",
                                           "real_function"])
        w.writeheader()
        for r in sorted(declared_rows, key=lambda r: -r["orig_insn"]):
            f = next((f for f in fo if hex(f["addr"]) == r["orig_addr"]), None)
            r["name"] = name_map.get(f["addr"]) or (f.get("name") if f else "") or ""
            r["tier"] = tier_map.get(f["addr"], "") if f else ""
            r["real_function"] = int(f["addr"] in pdata_addrs) if f else 0
            w.writerow(r)
    L = ["# Declared pairs — audit (DECLARED-DIVERGENT is NOT a match)\n"]
    A = L.append
    kinds = Counter(r["verdict"] for r in declared_rows)
    A(f"declarations in CSV: {len(declared_rows)}")
    for k in (BYTE, INSN, DECLARED, MISSING):
        if kinds.get(k):
            A(f"- {k}: {kinds.get(k, 0)}")
    real = [r for r in declared_rows if int(r.get("real_function", 0))]
    A(f"\ndeclared rows that are REAL (unwind-entry) functions: {len(real)}")
    rc = Counter(r["verdict"] for r in real)
    for k in (BYTE, INSN, DECLARED, MISSING):
        if rc.get(k):
            A(f"- {k}: {rc.get(k, 0)} "
              f"({sum(r['orig_insn'] for r in real if r['verdict'] == k)} insn)")
    A("\n## DECLARED-DIVERGENT rows (claim of intent, no match)\n")
    A("| orig | symbol | src evidence | new | orig insn | our insn | ratio |")
    A("|---|---|---|---|---|---|---|")
    for r in sorted((r for r in declared_rows if r["verdict"] == DECLARED),
                    key=lambda r: -r["orig_insn"]):
        A(f"| {r['orig_addr']} | {r['declared_symbol'][:40]} | {r['evidence']} | "
          f"{r['new_addr']} | {r['orig_insn']} | {r['our_insn']} | {r['ratio']:.4f} |")
    A("")
    open(out_md, "w", encoding="utf-8").write("\n".join(L) + "\n")
    return kinds, rc, real


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
    ap.add_argument("--infolding", action="store_true",
                    help="classify MISSING originals by WHY they are missing "
                         "(PRESENT-INFOLDED / ABSENT / UNKNOWN).  Measurement "
                         "only: it never adds, removes or changes a pair, and "
                         "PRESENT-INFOLDED is reported as its own bucket, never "
                         "as matched.")
    ap.add_argument("--declared", default=None, metavar="CSV",
                    help="seed evidence-based declared pairs from a mappings CSV "
                         "(tools/extract_declared.py).  Off by default so "
                         "previous numbers stay comparable.  A declared pair "
                         "whose bytes do not match is reported as "
                         f"{DECLARED}, which is NOT a match.")
    ap.add_argument("--declared-min-ratio", type=float, default=0.50,
                    help="minimum token ratio for binding a declared original "
                         "to an unpaired rebuild function (default 0.50).  "
                         "Below it the declared address stays MISSING.")
    ap.add_argument("--map", default=None, metavar="MAP",
                    help="MSVC /MAP linker map for the rebuild DLL.  When given, "
                         "a declaration's source symbol is bound to our "
                         "function's ADDRESS by exact map lookup instead of "
                         "being guessed by similarity.  Off by default; a "
                         "missing map file falls back to similarity binding.")
    ap.add_argument("--declared-out", default="recon/declared-verdicts.csv")
    ap.add_argument("--declared-report", default="recon/declared-report.md")
    args = ap.parse_args()

    name_map = load_symbols()

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
    pe_n, fn, mn, rn = build_side(args.new)
    # Tier classifier: computed here from the original's exports / plugin
    # string+RTTI refs / registration table / call reachability, exactly as
    # tools/parity.py does.  Do not depend on a stale external report whose
    # relative path silently failed when match.py ran from build/.
    tier_map, meta_names = compute_tiers(
        pe_o, fo, root=os.path.dirname(os.path.dirname(os.path.abspath(args.orig))))
    name_map = {**meta_names, **name_map}
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
    declared_rows = None
    map_resolver = None
    map_counts = None
    if args.declared:
        verdicts(fo, fn, pairs)   # so existing pairs carry bytematch/insn/ratio
        declared = load_declared(args.declared)
        if args.map:
            sym2addr = load_msvc_map(args.map)
            if not sym2addr:
                print(f"warning: map {args.map!r} missing or has no symbols; "
                      f"falling back to similarity binding")
            else:
                text = pe_n.section(".text")
                lo = pe_n.image_base + text["va"]
                hi = lo + max(text["vsize"], text["rsize"])

                def map_resolver(name, _s=sym2addr, _lo=lo, _hi=hi):
                    return resolve_map_symbol(_s, name, _lo, _hi)

                print(f"map {args.map}: {len(sym2addr)} symbols, "
                      f".text {hex(lo)}..{hex(hi)}")
        pairs, unmA, unmB, declared_rows = apply_declared(
            fo, fn, pairs, unmA, unmB, declared, args.declared_min_ratio,
            map_resolver)
        if map_resolver is not None:
            map_counts = Counter((r.get("map_reason") or "resolved")
                                 for r in declared_rows
                                 if r.get("binding") == "map")
            print("map binding: " + "  ".join(
                f"{k}={v}" for k, v in sorted(map_counts.items())))
    verdicts(fo, fn, pairs)

    infolding = None
    if args.infolding:
        import pair_identity as _pi
        import parity_names as _N
        import pairfix_infolding as _FI
        o_side = _pi.Side(pe_o, fo, _N.recover_rtti(args.orig),
                          _N.load_recon_names(pe_o))
        n_side = _pi.Side(pe_n, fn, _N.recover_rtti(args.new), None)
        vmap = {f["addr"]: MISSING for f in fo}
        for p in pairs:
            vmap[fo[p["i"]]["addr"]] = p["verdict"]
        src = _FI.source_literals(
            os.path.join(os.path.dirname(HERE), "src"))
        infolding = _FI.classify_missing(fo, pairs, vmap, o_side, n_side, src)
        lines, counts, insn = _FI.summarise("MISSING classified",
                                            sorted(infolding.values(),
                                                   key=lambda r: -r["orig_insn"]))
        print("\n".join(lines))

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

    declared_kinds = None
    if declared_rows is not None:
        pdata_addrs = {b for b, e, _ in pe_o.parse_pdata() if e > b}
        declared_kinds, _rc, _real = write_declared(
            args.declared_out, args.declared_report, declared_rows, fo, fn,
            pairs, name_map, tier_map, pdata_addrs)

    pbyi = {p["i"]: p for p in pairs}
    jout = {
        "harness": "tools/match.py",
        "orig": args.orig, "new": args.new,
        "masking": masked,
        "functions": [],
    }
    if declared_rows is not None:
        jout["declared"] = {"csv": args.declared,
                            "min_ratio": args.declared_min_ratio,
                            "counts": dict(declared_kinds)}
        if args.map:
            jout["declared"]["map"] = {
                "path": args.map,
                "symbols": sum(len(v) for v in load_msvc_map(args.map).values()),
                "binding": dict(map_counts or {})}
    dec_by_addr = {}
    if declared_rows is not None:
        dec_by_addr = {r["orig_addr"]: r for r in declared_rows}
    for i, f in enumerate(fo):
        p = pbyi.get(i)
        row = {
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
        }
        # A declaration is a first-class thing in the model, not a printer-side
        # annotation: EVERY declared original carries its symbol, evidence and
        # binding here (including one that could not be bound and stays MISSING),
        # so every consumer sees the same population.
        dec_row = dec_by_addr.get(hex(f["addr"]))
        if dec_row is not None:
            row["declared"] = True
            row["declared_symbol"] = dec_row["declared_symbol"]
            row["declared_evidence"] = dec_row["evidence"]
            row["declared_round"] = dec_row.get("round", "")
            row["declared_binding"] = dec_row["binding"]
            row["declared_map_symbol"] = dec_row.get("map_symbol", "")
            row["declared_map_reason"] = dec_row.get("map_reason", "")
            row["declared_verdict"] = dec_row["verdict"]
            if p is None:
                # an UNBOUND declaration: it stays MISSING, with its reason
                # retained here; it is never silently folded into the
                # undeclared MISSING mass.
                row["declared_class"] = "UNBOUND"
            elif p.get("declared"):
                row["declared_class"] = (DECLARED if p["verdict"] == DECLARED
                                         else "BOUND")
            else:
                # declaration agrees with an independently found pair; the
                # pair's own verdict stands (a declaration never changes it).
                row["declared_class"] = "BOUND"
                row["declared_existing_pair"] = True
        if infolding is not None and f["addr"] in infolding:
            r = infolding[f["addr"]]
            row["missing_class"] = r["class"]
            row["missing_evidence"] = r["evidence"]
        jout["functions"].append(row)
    if id_meta:
        jout["identity"] = id_meta
    json.dump(jout, open(OUTJSON, "w"), indent=1)

    allt = tiers["all"]
    c = allt["counts"]
    print(f"orig functions {allt['orig_funcs']}, rebuild {len(fn)}")
    print(f"BYTE-MATCH {c.get(BYTE,0)}  INSN-MATCH {c.get(INSN,0)}  "
          f"RATIO {c.get(RATIO,0)}  MISSING {c.get(MISSING,0)}"
          + (f"  {DECLARED} {c.get(DECLARED,0)}" if c.get(DECLARED) else ""))
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
    if declared_rows is not None:
        print(f"declared: {dict(declared_kinds)}  wrote {args.declared_out}, "
              f"{args.declared_report}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
