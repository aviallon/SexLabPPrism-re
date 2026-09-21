#!/usr/bin/env python3
"""Identity-aware function pairing for the SexLabPPrism matching decompilation.

Structurally pairing two independently-linked DLLs is unreliable: our rebuild
places a function at a different address, inlines differently, and the linker
may fold identical bodies.  This tool pairs ORIGINAL functions to their TRUE
counterparts using identity anchors, in priority order:

  1. NAMES        exact recovered-name equality (embedded __FUNCSIG__ / RTTI
                  vtable slots / recon aliases)
  2. ALIASES      an explicit, printed table of our renaming vs the original's
                  recovered names (Papyrus natives, bridge classes)
  3. STRINGS      a function that references a distinctive literal is anchored
                  to the original function referencing the same literal
  4. CALL GRAPH   a function whose callees/callers are already paired is likely
                  the counterpart of X (iterative mutual-best propagation)
  5. STRUCTURE    strict fallback (same instruction count, high mnemonic and
                  token similarity, mutual best) -- LOW confidence, excluded
                  from the headline number.

Pairing is bipartite (one rebuild function cannot absorb many originals) with
an explicit ICF pass for linker-folded identical bodies.

All PE parsing, disassembly, inventory and normalisation are imported from
`tools/parity.py` and `tools/parity_names.py`; nothing is re-implemented here.

Usage:
  python3 tools/pair_identity.py --orig artifacts/SexLabPPrism.dll \
      --new artifacts/rebuild/SexLabPPrism-parity-lto2.dll \
      --out recon/identity-matching.json --report recon/identity-matching.md
  python3 tools/pair_identity.py --self-test
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
from collections import Counter, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import parity  # noqa: E402  (shared machinery)
import parity_names as N  # noqa: E402

P = parity

# ---------------------------------------------------------------------------
# explicit alias table: OUR recovered name -> ORIGINAL recovered name
# Every entry is printed in the report; nothing is silent.  `evidence` says how
# the equivalence was established.
# ---------------------------------------------------------------------------
ALIASES = {
    "SceneState::BeginSceneSession": ("`anonymous-namespace'::Papyrus_BeginSceneSession",
                                      "Papyrus native registration order/contract"),
    "Catalog::Begin": ("`anonymous-namespace'::Papyrus_CatalogBegin",
                       "Papyrus native registration order/contract"),
    "Catalog::Finish": ("`anonymous-namespace'::Papyrus_CatalogFinish",
                        "Papyrus native registration order/contract"),
    "Catalog::Package": ("`anonymous-namespace'::Papyrus_CatalogPackage",
                         "Papyrus native registration order/contract"),
    "Catalog::Publish": ("`anonymous-namespace'::Papyrus_CatalogPublish",
                         "Papyrus native registration order/contract"),
    "SceneState::Publish": ("`anonymous-namespace'::Papyrus_PublishSceneState",
                            "Papyrus native registration order/contract"),
    "SceneState::PublishCompatible": ("`anonymous-namespace'::Papyrus_PublishCompatible",
                                      "Papyrus native registration order/contract"),
    "SceneState::SetSearchQuery": ("`anonymous-namespace'::Papyrus_SetSearchQuery",
                                   "Papyrus native registration order/contract"),
    "Papyrus::Natives::Log": ("`anonymous-namespace'::Papyrus_Log",
                              "Papyrus native registration order/contract"),
    "ActionDispatch::SendModEvent": ("`anonymous-namespace'::SendAction",
                                     "sends the action ModEvent; string/semantic"),
    "Presentation::ApplyVanillaHUDVisibility": ("`anonymous-namespace'::ApplyPresentation",
                                                "applies presentation state; semantic"),
    "InputSink::`anonymous-namespace'::Sink::ProcessEvent":
        ("`anonymous-namespace'::InputSink::ProcessEvent",
         "our Sink::ProcessEvent is the InputSink vtable method"),
    "MenuVisibilitySink::`anonymous-namespace'::Handle":
        ("`anonymous-namespace'::MenuVisibilitySink::ProcessEvent",
         "our Handle is the MenuVisibilitySink vtable method"),
}

NAME, ALIAS, RTTI, STRING, CALLGRAPH, STRUCT, ICF = (
    "name", "alias", "rtti", "string", "callgraph", "structural", "icf")
HIGH_CONF = (NAME, ALIAS, RTTI, STRING, CALLGRAPH, ICF)


# ---------------------------------------------------------------------------
# side preparation (names, strings, call graph)
# ---------------------------------------------------------------------------

def _string_map(pe):
    """VA -> literal for printable runs outside .text."""
    tsec = pe.section(".text")
    tlo = pe.image_base + tsec["va"]
    thi = tlo + tsec["vsize"]
    out = {}
    for m in re.finditer(rb"[\x20-\x7e]{4,}", pe.data):
        rva = pe.off2rva(m.start())
        if rva is None:
            continue
        va = pe.image_base + rva
        if tlo <= va < thi:
            continue
        out[va] = m.group().decode("latin1")
    return out


def _call_map(funcs, starts):
    out = {}
    for f in funcs:
        out[f["addr"]] = {i["branch"] for i in f["insns"]
                          if i["mn"] == "call" and i["branch"] in starts}
    return out


def _callers(callees):
    inv = defaultdict(set)
    for a, cs in callees.items():
        for c in cs:
            inv[c].add(a)
    return inv


class Side:
    def __init__(self, pe, funcs, rtti, extra_names=None):
        self.pe = pe
        self.funcs = funcs
        self.by_addr = {f["addr"]: f for f in funcs}
        self.starts = set(self.by_addr)
        self.addr_to_names, self.name_to_addrs, self.kind = N.recover_names(
            pe, funcs, rtti, extra_names)
        self.smap = _string_map(pe)
        self.litrefs = {}
        for f in funcs:
            self.litrefs[f["addr"]] = frozenset(
                self.smap[r] for r in f["refs"] if r in self.smap)
        self.callees = _call_map(funcs, self.starts)
        self.callers = _callers(self.callees)


def build_side(path, rtti_json=None):
    pe, funcs, _meta = N.build_side(path, P)
    if rtti_json is None:
        rtti_json = N.recover_rtti(path)
    extra = N.load_recon_names(pe) if path.endswith("SexLabPPrism.dll") else None
    return Side(pe, funcs, rtti_json, extra)


# ---------------------------------------------------------------------------
# anchors 1-5
# ---------------------------------------------------------------------------

def _unique(map_, key):
    s = map_.get(key)
    if s and len(s) == 1:
        return next(iter(s))
    return None


def anchor_names(o: Side, n: Side):
    """Exact recovered-name pairs (name -> unique function on both sides).

    `__FUNCSIG__` names are the strongest (the compiler stamped the source
    signature); RTTI vtable-slot names are kept as a separate `rtti` anchor so
    the report can state how many pairs each evidence type contributed."""
    out, rtti_out = {}, {}
    matched, ambiguous, _, _ = N.pair_by_name(o.name_to_addrs, n.name_to_addrs)
    for nm, oa, na in matched:
        k = o.kind.get(nm) or n.kind.get(nm) or ""
        if k.startswith("rtti"):
            rtti_out.setdefault(oa, {})[na] = (RTTI, nm)
        else:
            out.setdefault(oa, {})[na] = (NAME, nm)
    return out, rtti_out


def anchor_aliases(o: Side, n: Side, aliases=ALIASES):
    out, unresolved = {}, []
    for our, (orig, evidence) in aliases.items():
        oa, na = _unique(o.name_to_addrs, orig), _unique(n.name_to_addrs, our)
        if oa is not None and na is not None:
            out.setdefault(oa, {})[na] = (ALIAS, f"{our} -> {orig}")
        else:
            unresolved.append({"our": our, "orig": orig, "evidence": evidence,
                               "orig_found": oa is not None, "new_found": na is not None})
    return out, unresolved


def anchor_strings(o: Side, n: Side):
    """Distinctive literal referenced by exactly one function per side."""
    lit_o = defaultdict(set)
    lit_n = defaultdict(set)
    for f in o.funcs:
        for lit in o.litrefs[f["addr"]]:
            lit_o[lit].add(f["addr"])
    for f in n.funcs:
        for lit in n.litrefs[f["addr"]]:
            lit_n[lit].add(f["addr"])
    votes = defaultdict(lambda: [0, None])
    for lit, aos in lit_o.items():
        ans = lit_n.get(lit)
        if not ans:
            continue
        if len(aos) == 1 and len(ans) == 1:
            oa, na = next(iter(aos)), next(iter(ans))
            w = min(len(lit), 80)
            votes[(oa, na)][0] += w
            if votes[(oa, na)][1] is None:
                votes[(oa, na)][1] = lit
    out = {}
    for (oa, na), (w, lit) in votes.items():
        out.setdefault(oa, {})[na] = (STRING, f"{w} chars: {lit[:60]!r}")
    return out


def _mutual_best(votes, used_o, used_n, min_score=1):
    """votes: {(oa,na): (score, note)} -> mutually-best free pairs."""
    best_o, best_n = {}, {}
    for (oa, na), (sc, note) in votes.items():
        if oa in used_o or na in used_n:
            continue
        if oa not in best_o or sc > best_o[oa][0]:
            best_o[oa] = (sc, na, note)
        if na not in best_n or sc > best_n[na][0]:
            best_n[na] = (sc, oa, note)
    out = {}
    for oa, (sc, na, note) in best_o.items():
        if best_n.get(na, (None, None))[1] == oa and sc >= min_score:
            out[oa] = (na, sc, note)
    return out


def anchor_callgraph(o: Side, n: Side, pairs, rounds=4):
    """Iterative mutual-best propagation over already-paired callees/callers."""
    o2n = dict(pairs)
    n2o = {v: k for k, v in o2n.items()}
    added = {}
    for _ in range(rounds):
        votes = defaultdict(lambda: [0, None])
        for oa, na in list(o2n.items()):
            for oc in o.callers.get(oa, ()):
                for nc in n.callers.get(na, ()):
                    if oc in o2n or nc in n2o:
                        continue
                    votes[(oc, nc)][0] += 1
                    if votes[(oc, nc)][1] is None:
                        votes[(oc, nc)][1] = "caller"
            for ok in o.callees.get(oa, ()):
                for nk in n.callees.get(na, ()):
                    if ok in o2n or nk in n2o:
                        continue
                    votes[(ok, nk)][0] += 1
                    if votes[(ok, nk)][1] is None:
                        votes[(ok, nk)][1] = "callee"
        if not votes:
            break
        mb = _mutual_best(votes, set(o2n), n2o, min_score=2)
        if not mb:
            break
        for oa, (na, sc, note) in mb.items():
            o2n[oa] = na
            n2o[na] = oa
            added[oa] = {na: (CALLGRAPH, f"{note} votes={sc}")}
    return added


def anchor_structure(o: Side, n: Side, used_o, used_n, min_ratio=0.90):
    """Strict structural fallback.  Only both-unnamed functions, near-identical
    instruction count, high token ratio and high mnemonic overlap, mutual best.
    LOW confidence: recorded, never used for the headline number."""
    cand_o = [f for f in o.funcs if f["addr"] not in used_o and not o.addr_to_names.get(f["addr"])]
    cand_n = [f for f in n.funcs if f["addr"] not in used_n and not n.addr_to_names.get(f["addr"])]
    # index new by rounded instruction count
    idx = defaultdict(list)
    for g in cand_n:
        idx[g["icount"]].append(g)
    votes = {}
    for f in cand_o:
        na, nb = f["icount"], None
        if na < 4:
            continue
        pool = []
        for c in range(na - max(1, na // 20), na + max(1, na // 20) + 1):
            pool.extend(idx.get(c, ()))
        for g in pool:
            tot = max(f["mnem"].total(), g["mnem"].total(), 1)
            inter = sum((f["mnem"] & g["mnem"]).values())
            if inter / tot < 0.85:
                continue
            r = N.ratio_for(f, g)[0]
            if r < min_ratio:
                continue
            votes[(f["addr"], g["addr"])] = (r, f"ratio={r:.3f}")
    mb = _mutual_best(votes, set(), {})
    out = {}
    for oa, (na, sc, note) in mb.items():
        out.setdefault(oa, {})[na] = (STRUCT, note)
    return out


def anchor_icf(o: Side, n: Side, pairs):
    """Linker-folded identical bodies: an unpaired original whose normalised
    token stream equals an already-paired (or unpaired) rebuild function."""
    by_hash_n = defaultdict(list)
    for f in n.funcs:
        by_hash_n[f["strict_hash"]].append(f["addr"])
    out = {}
    already_n = set(pairs.values())
    for f in o.funcs:
        if f["addr"] in pairs:
            continue
        # tiny bodies are only trusted between two real .pdata functions
        if f["icount"] < 3 and f.get("source") != "pdata":
            continue
        for na in by_hash_n.get(f["strict_hash"], ()):
            # accept a fold even if the rebuild function is already used
            out.setdefault(f["addr"], {})[na] = (ICF, "identical normalised body")
            break
    return out


def merge(objs):
    """Merge anchor dicts {oa:{na:(conf,note)}} into a flat best map."""
    out = {}
    for d in objs:
        for oa, ns in d.items():
            for na, (conf, note) in ns.items():
                cur = out.get(oa)
                if cur is None or (na != cur[0]):
                    # first writer wins across priority-ordered anchors
                    out.setdefault(oa, (na, conf, note))
    return out


# ---------------------------------------------------------------------------
# driver
# ---------------------------------------------------------------------------

ANCHOR_ORDER = (NAME, ALIAS, RTTI, STRING, CALLGRAPH, ICF, STRUCT)


def pair_sides(o: Side, n: Side, aliases=ALIASES, strict_ratio=0.90):
    """Return (pairs, unmA, unmB, meta).  pairs: list of dicts with
    orig_addr/new_addr/anchor/confidence/note."""
    used_o, used_n = set(), set()
    pairs = []

    def take(d, confidence, allow_shared=False):
        # bipartite greedy: within a pass, one-to-one only (ICF may share)
        for oa in sorted(d):
            if oa in used_o:
                continue
            ns = d[oa]
            for na in sorted(ns):
                if na in used_n and not allow_shared:
                    continue
                conf, note = ns[na]
                pairs.append({"orig_addr": oa, "new_addr": na,
                              "anchor": confidence, "confidence": confidence,
                              "note": note})
                used_o.add(oa)
                used_n.add(na)
                break

    # 1. exact names (funcsig), then RTTI vtable slots
    name_pairs, rtti_pairs = anchor_names(o, n)
    take(name_pairs, NAME)
    take(rtti_pairs, RTTI)
    # 2. explicit aliases
    alias_pairs, alias_unresolved = anchor_aliases(o, n, aliases)
    take(alias_pairs, ALIAS)
    # 3. strings (only unambiguous literals)
    take(anchor_strings(o, n), STRING)
    # 4. ICF folds (byte/token-identical bodies are definitive evidence, so
    #    this runs BEFORE the softer call-graph propagation)
    o2n = {p["orig_addr"]: p["new_addr"] for p in pairs}
    take(anchor_icf(o, n, o2n), ICF, allow_shared=True)
    # 5. call-graph propagation (mutual best, >=2 votes)
    o2n = {p["orig_addr"]: p["new_addr"] for p in pairs}
    take(anchor_callgraph(o, n, o2n), CALLGRAPH)
    # 6. strict structural, low confidence
    take(anchor_structure(o, n, used_o, used_n, strict_ratio), STRUCT)

    unmA = [f["addr"] for f in o.funcs if f["addr"] not in used_o]
    unmB = [f["addr"] for f in n.funcs if f["addr"] not in used_n]
    meta = {
        "anchors": dict(Counter(p["anchor"] for p in pairs)),
        "alias_unresolved": alias_unresolved,
        "aliases": {k: v[0] for k, v in aliases.items()},
        "structural_min_ratio": strict_ratio,
    }
    return pairs, unmA, unmB, meta


def pair_indices(fo, fn, orig_path, new_path, per_function=None, aliases=ALIASES):
    """Convenience for match.py: takes already-built funcs, returns pairs keyed
    by list index with extra anchor keys."""
    tmap = {}
    if per_function and os.path.exists(per_function):
        try:
            doc = json.load(open(per_function))
            for r in doc.get("orig_functions", []):
                if r.get("tier"):
                    tmap[int(r["addr"], 16)] = r["tier"]
        except Exception:
            pass
    o = Side(P.PE(orig_path), fo, {})
    # recover names only (skip expensive string/call maps? need them)
    o.addr_to_names, o.name_to_addrs, o.kind = N.recover_names(
        o.pe, fo, N.recover_rtti(orig_path), N.load_recon_names(o.pe))
    o.smap = _string_map(o.pe)
    o.litrefs = {f["addr"]: frozenset(o.smap[r] for r in f["refs"] if r in o.smap)
                 for f in fo}
    o.callees = _call_map(fo, o.starts)
    o.callers = _callers(o.callees)
    n = Side(P.PE(new_path), fn, {})
    n.addr_to_names, n.name_to_addrs, n.kind = N.recover_names(
        n.pe, fn, N.recover_rtti(new_path), None)
    n.smap = _string_map(n.pe)
    n.litrefs = {f["addr"]: frozenset(n.smap[r] for r in f["refs"] if r in n.smap)
                 for f in fn}
    n.callees = _call_map(fn, n.starts)
    n.callers = _callers(n.callees)
    pairs, unmA, unmB, meta = pair_sides(o, n, aliases)
    oi = {f["addr"]: i for i, f in enumerate(fo)}
    ni = {f["addr"]: j for j, f in enumerate(fn)}
    ip = []
    for p in pairs:
        if p["orig_addr"] in oi and p["new_addr"] in ni:
            q = dict(p)
            q["i"] = oi[p["orig_addr"]]
            q["j"] = ni[p["new_addr"]]
            q["ratio"] = N.ratio_for(fo[q["i"]], fn[q["j"]])[0]
            ip.append(q)
    return ip, [oi[a] for a in unmA if a in oi], [ni[a] for a in unmB if a in ni], meta


# ---------------------------------------------------------------------------
# self-test (synthetic inventories; fast and deterministic)
# ---------------------------------------------------------------------------

def _mkf(addr, tokens, names=(), refs=(), calls=()):
    f = {"addr": addr, "end": addr + max(1, len(tokens)), "size": len(tokens),
         "tokens": list(tokens), "raw_tokens": list(tokens), "icount": len(tokens),
         "mnem": Counter(t.split(" ", 1)[0] for t in tokens),
         "strict_hash": P.sha1("\n".join(tokens)),
         "raw_hash": P.sha1("\n".join(tokens)), "insns": [], "refs": set(refs)}
    for c in calls:
        f["insns"].append({"addr": addr, "size": 1, "mn": "call", "branch": c})
    return f


class _FakePE:
    image_base = 0x180000000

    def section(self, _):
        return {"va": 0, "vsize": 0x1000}

    def off2rva(self, _):
        return None


class _FakeSide(Side):
    def __init__(self, funcs, names):
        # bypass binary parsing
        self.pe = _FakePE()
        self.funcs = funcs
        self.by_addr = {f["addr"]: f for f in funcs}
        self.starts = set(self.by_addr)
        self.addr_to_names = defaultdict(set)
        self.name_to_addrs = defaultdict(set)
        for a, ns in names.items():
            for nm in ns:
                self.addr_to_names[a].add(nm)
                self.name_to_addrs[nm].add(a)
        self.kind = {}
        self.smap = {}
        self.litrefs = {f["addr"]: frozenset(f["refs"]) for f in funcs}
        self.callees = _call_map(funcs, self.starts)
        self.callers = _callers(self.callees)


def self_test():
    notes = []
    passed = total = 0

    def check(cond, msg):
        nonlocal passed, total
        total += 1
        passed += 1 if cond else 0
        notes.append(("PASS " if cond else "FAIL ") + msg)

    # --- exact name anchor ---
    A = [_mkf(0x100, ["push rbp", "mov eax,0x1", "pop rbp", "ret"])]
    B = [_mkf(0x900, ["push rbp", "mov eax,0x1", "pop rbp", "ret"])]
    pairs, _, _, _ = pair_sides(_FakeSide(A, {0x100: ["ns::Foo"]}),
                                _FakeSide(B, {0x900: ["ns::Foo"]}))
    check(len(pairs) == 1 and pairs[0]["anchor"] == NAME,
          f"exact name anchor finds 0x100->0x900 ({pairs[0]['anchor'] if pairs else 'none'})")

    # --- renamed symbol tables: alias anchor still resolves ---
    A = [_mkf(0x100, ["push rbp", "mov eax,0x1", "pop rbp", "ret"])]
    B = [_mkf(0x900, ["push rbp", "mov eax,0x1", "pop rbp", "ret"])]
    renamed = {"ours::Real": ("original::Wrong", "renamed-symbol-table test")}
    pairs, _, _, meta = pair_sides(_FakeSide(A, {0x100: ["original::Wrong"]}),
                                   _FakeSide(B, {0x900: ["ours::Real"]}),
                                   aliases=renamed)
    check(len(pairs) == 1 and pairs[0]["anchor"] == ALIAS,
          "renamed symbol tables: alias anchor recovers 0x100->0x900")

    # --- string anchor with BOTH sides renamed (no name anchor possible) ---
    A = [_mkf(0x200, ["lea rax", "call EXT", "ret"], refs=("Catalog incomplete on UI side",))]
    B = [_mkf(0xa00, ["push rbx", "lea rcx", "call EXT", "pop rbx", "ret"],
              refs=("Catalog incomplete on UI side",))]
    pairs, _, _, _ = pair_sides(_FakeSide(A, {}), _FakeSide(B, {}))
    check(len(pairs) == 1 and pairs[0]["anchor"] == STRING,
          "string anchor pairs functions by shared distinctive literal")

    # --- call-graph anchor (>=2 votes) ---
    A = [_mkf(0x300, ["call 0x400", "call 0x440", "ret"], calls=(0x400, 0x440)),
         _mkf(0x400, ["mov eax,eax", "ret"]),
         _mkf(0x440, ["xor ecx,ecx", "ret"])]
    B = [_mkf(0xb00, ["call 0xc00", "call 0xc40", "ret"], calls=(0xc00, 0xc40)),
         _mkf(0xc00, ["mov eax,eax", "ret"]),
         _mkf(0xc40, ["xor ecx,ecx", "ret"])]
    pairs, _, _, _ = pair_sides(_FakeSide(A, {0x400: ["ns::C1"], 0x440: ["ns::C2"]}),
                                _FakeSide(B, {0xc00: ["ns::C1"], 0xc40: ["ns::C2"]}))
    byo = {p["orig_addr"]: p for p in pairs}
    check(0x300 in byo and byo[0x300]["anchor"] == CALLGRAPH,
          f"call-graph propagation pairs the caller (0x300 -> "
          f"{hex(byo[0x300]['new_addr']) if 0x300 in byo else 'none'})")

    # --- wrong structural pair must be REJECTED ---
    A = [_mkf(0x500, ["mov eax,0x1", "add eax,0x2", "imul eax,0x3",
                      "sub eax,0x4", "xor ecx,ecx", "shl eax,cl", "ret"])]
    B = [_mkf(0xd00, ["movups xmm0,[rcx]", "addps xmm0,xmm1", "movups [rdx],xmm0",
                      "cvtsi2ss xmm2,eax", "mulss xmm2,xmm2", "movaps xmm3,xmm0",
                      "ret"])]
    pairs, _, _, _ = pair_sides(_FakeSide(A, {}), _FakeSide(B, {}))
    check(len(pairs) == 0, "dissimilar functions are NOT structurally paired (wrong pair rejected)")

    # --- ICF fold: two originals identical, one rebuild body -> 2 matches ---
    body = ["push rbp", "mov eax,0x1", "mov ecx,0x2", "add eax,ecx",
            "pop rbp", "ret"]
    A = [_mkf(0x600, body), _mkf(0x680, body)]
    B = [_mkf(0xe00, body)]
    pairs, umA, _, _ = pair_sides(_FakeSide(A, {}), _FakeSide(B, {}))
    check(len(pairs) == 2 and not umA and
          any(p["anchor"] == ICF for p in pairs),
          f"ICF fold: 2 originals share 1 rebuild body (pairs={len(pairs)}, "
          f"anchors={[p['anchor'] for p in pairs]})")

    # --- bipartite: one rebuild cannot absorb two different originals ---
    A = [_mkf(0x700, ["push rbp", "mov eax,0x1", "pop rbp", "ret"]),
         _mkf(0x780, ["push rbp", "mov eax,0x2", "pop rbp", "ret"])]
    B = [_mkf(0xf00, ["push rbp", "mov eax,0x1", "pop rbp", "ret"])]
    pairs, _, _, _ = pair_sides(_FakeSide(A, {}), _FakeSide(B, {}))
    check(len(pairs) == 1 and len({p["new_addr"] for p in pairs}) == 1,
          "bipartite: one rebuild body matches at most one non-identical original")

    return passed, total, notes


# ---------------------------------------------------------------------------
# report
# ---------------------------------------------------------------------------

def write_report(path, orig, new, o, n, pairs, unmA, unmB, meta, before=None):
    L = []
    A = L.append
    A("# Identity-aware pairing - SexLabPPrism 0.6.1")
    A("")
    A("Generated by `tools/pair_identity.py`. Pairs ORIGINAL functions to their")
    A("true rebuild counterparts by identity anchors (names/aliases/strings/")
    A("call-graph), with a strict structural fallback marked LOW confidence and")
    A("excluded from the headline. Machinery imported from `tools/parity.py` and")
    A("`tools/parity_names.py`.")
    A("")
    A(f"Original: `{orig}`  |  Rebuild: `{new}`")
    A("")
    A("## Anchor summary")
    A("")
    A("| anchor | pairs | headline |")
    A("|---|---|---|")
    for a in ANCHOR_ORDER:
        c = meta["anchors"].get(a, 0)
        A(f"| {a} | {c} | {'yes' if a in HIGH_CONF else 'LOW (excluded)'} |")
    A(f"| **total** | **{len(pairs)}** | |")
    A("")
    hl = sum(1 for p in pairs if p["confidence"] in HIGH_CONF)
    A(f"High-confidence pairs (anchor 1-4 + ICF): **{hl}**; "
      f"structural-only (low confidence): **{len(pairs)-hl}**.")
    A(f"Unmatched: original **{len(unmA)}**, rebuild **{len(unmB)}**.")
    A("")
    A("## Alias table (explicit; every equivalence is stated)")
    A("")
    A("| our recovered name | original recovered name | resolved | evidence |")
    A("|---|---|---|---|")
    unres = {u["our"]: u for u in meta.get("alias_unresolved", [])}
    for our, orig_nm in meta["aliases"].items():
        u = unres.get(our)
        ev = u["evidence"] if u else "resolved"
        ok = "yes" if u is None else (
            f"NO (orig={'ok' if u['orig_found'] else '-'}, new={'ok' if u['new_found'] else '-'})")
        A(f"| `{our}` | `{orig_nm}` | {ok} | {ev} |")
    A("")
    A("## Top convergence targets (faithfully paired, not yet byte-equal)")
    A("")
    A("Ranked by ratio. `anchor` is how the pair was identified; `struct` rows are")
    A("low confidence and do not count toward the headline.")
    A("| ratio | anchor | orig addr | new addr | orig insn | new insn | name |")
    A("|---|---|---|---|---|---|---|")
    rows = []
    for p in pairs:
        fa, fb = o.by_addr.get(p["orig_addr"]), n.by_addr.get(p["new_addr"])
        if not fa or not fb:
            continue
        r = N.ratio_for(fa, fb)[0]
        if r >= 1.0:
            continue
        nm = ", ".join(sorted(o.addr_to_names.get(p["orig_addr"], ())))[:60] or "-"
        rows.append((r, p, fa, fb, nm))
    rows.sort(key=lambda x: -x[0])
    for r, p, fa, fb, nm in rows[:60]:
        A(f"| {r:.3f} | {p['anchor']} | 0x{p['orig_addr']:x} | 0x{p['new_addr']:x} | "
          f"{fa['icount']} | {fb['icount']} | {nm} |")
    A("")
    A("## What this pairing still cannot see")
    A("")
    A("- Unnamed private leaf helpers with no literal and no RTTI slot: only the")
    A("  strict structural fallback can reach them, and that is LOW confidence.")
    A("- A wrong NAME can still mispair: recovered names come from `__FUNCSIG__`")
    A("  and RTTI, so a function that merely logs another function's signature")
    A("  is mis-named. Aliases are hand-maintained and stated above.")
    A("- ICF/thunks: identical bodies are intentionally shared; they are counted")
    A("  as pairs under the `icf` anchor and listed separately, not silently.")
    A("- The strict structural fallback is deliberately conservative (>=0.90")
    A("  token ratio, same instruction count, mutual best). It REJECTS rather")
    A("  than guesses, so some true counterparts remain MISSING.")
    A("")
    open(path, "w", encoding="utf-8").write("\n".join(L) + "\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--orig", default="artifacts/SexLabPPrism.dll")
    ap.add_argument("--new", default="artifacts/rebuild/SexLabPPrism-parity-lto2.dll")
    ap.add_argument("--out", default="recon/identity-matching.json")
    ap.add_argument("--report", default="recon/identity-matching.md")
    ap.add_argument("--self-test", action="store_true")
    ap.add_argument("--from-json", action="store_true")
    args = ap.parse_args()

    if args.self_test:
        passed, total, notes = self_test()
        print(f"self-test: {passed}/{total} passed")
        for nte in notes:
            print("  " + nte)
        return 0 if passed == total else 1

    if args.from_json and os.path.exists(args.out):
        d = json.load(open(args.out))
        print(f"read {len(d['pairs'])} pairs from {args.out}")
        return 0

    rtti_o = N.recover_rtti(args.orig)
    rtti_n = N.recover_rtti(args.new)
    o = build_side(args.orig, rtti_o)
    n = build_side(args.new, rtti_n)
    pairs, unmA, unmB, meta = pair_sides(o, n)

    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    json.dump({
        "orig": args.orig, "new": args.new,
        "anchors": meta["anchors"], "aliases": meta["aliases"],
        "alias_unresolved": meta["alias_unresolved"],
        "pairs": [{"orig": hex(p["orig_addr"]), "new": hex(p["new_addr"]),
                   "anchor": p["anchor"], "confidence": p["confidence"],
                   "note": p["note"]} for p in pairs],
        "unmatched_orig": [hex(a) for a in unmA],
        "unmatched_new": [hex(a) for a in unmB],
    }, open(args.out, "w"), indent=1)
    write_report(args.report, args.orig, args.new, o, n, pairs, unmA, unmB, meta)

    print(f"funcs: orig {len(o.funcs)}, rebuild {len(n.funcs)}")
    print("anchor pairs: " + " ".join(
        f"{a}={meta['anchors'].get(a,0)}" for a in ANCHOR_ORDER))
    hl = sum(1 for p in pairs if p["confidence"] in HIGH_CONF)
    print(f"pairs {len(pairs)} (high-confidence {hl}, structural {len(pairs)-hl})")
    print(f"unmatched orig {len(unmA)}, rebuild {len(unmB)}")
    print(f"wrote {args.out}, {args.report}")
    return 0


if __name__ == "__main__":
    sys.exit(main())