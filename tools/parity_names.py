#!/usr/bin/env python3
"""Name-aware disassembly parity for two Windows x64 PEs.

`tools/parity.py` pairs functions structurally (name-blind).  This tool
recovers a real name -> function map on BOTH binaries and pairs them BY NAME,
then explains, per pair, the first divergence and a unified diff of the
normalised token streams.  It answers the question the structural harness
cannot: "we implemented this function -- which original function is it, and
what exactly differs?"

Name sources (both recovered per binary, no symbols required):
  * embedded `__FUNCSIG__` literals (spdlog / CLNG source_location macros
    emit them at every log call site) -> the function that references the
    literal address;
  * MSVC RTTI (reused from `tools/pe_rtti.py`): TypeDescriptors, and the
    virtual-function pointers stored in each class vtable.  A vtable slot is
    a direct function address, so it names the virtual methods.

All PE parsing / disassembly / inventory / normalisation is IMPORTED from
`tools/parity.py`; nothing is duplicated here.  Extra dependency: objdump.

Usage:
  python3 tools/parity_names.py \
      --orig artifacts/SexLabPPrism.dll \
      --new  artifacts/rebuild/SexLabPPrism-rebuild-v2.dll \
      --report recon/parity-names-v2.md
"""
from __future__ import annotations

import argparse
import bisect
import json
import os
import re
import struct
import subprocess
import sys
import tempfile
from collections import Counter, defaultdict
from difflib import SequenceMatcher, unified_diff

HERE = os.path.dirname(os.path.abspath(__file__))


def load_parity(path="tools/parity.py"):
    import importlib.util
    path = path if os.path.exists(path) else os.path.join(HERE, "parity.py")
    spec = importlib.util.spec_from_file_location("parity_harness", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


P = load_parity()

# ---------------------------------------------------------------------------
# name extraction
# ---------------------------------------------------------------------------

CC_RE = re.compile(r"\b__(?:cdecl|stdcall|thiscall|fastcall|vectorcall)\b")
FUNCSIG_MARKERS = (b"__cdecl", b"__stdcall", b"__thiscall",
                   b"__fastcall", b"__vectorcall")
PRINTABLE_RUN = re.compile(rb"[\x20-\x7e]{6,}")


def canonical_funcsig_name(s: str):
    """'bool __cdecl REL::Module::init(void)' -> 'REL::Module::init'."""
    m = CC_RE.search(s)
    if not m:
        return None
    rest = s[m.end():].lstrip()
    if not rest:
        return None
    i = rest.find("(")
    while i != -1 and rest[:i].rstrip().endswith("operator"):
        j = rest.find(")", i)
        i = rest.find("(", j + 1) if j != -1 else -1
    name = rest[:i] if i != -1 else rest
    name = re.sub(r"\s+", " ", name).strip()
    name = name.replace("operator ()", "operator()")
    return name or None


def extract_funcsigs(pe):
    """{literal VA -> canonical name} for every embedded __FUNCSIG__."""
    out = {}
    named = 0
    for m in PRINTABLE_RUN.finditer(pe.data):
        raw = m.group()
        if not any(k in raw for k in FUNCSIG_MARKERS):
            continue
        rva = pe.off2rva(m.start())
        if rva is None:
            continue
        nm = canonical_funcsig_name(raw.decode("latin1", "replace"))
        if not nm:
            continue
        out[pe.image_base + rva] = nm
        named += 1
    return out


def recover_rtti(dll_path):
    """Reuse tools/pe_rtti.py verbatim in a scratch cwd; return its JSON."""
    with tempfile.TemporaryDirectory() as td:
        os.makedirs(os.path.join(td, "recon"), exist_ok=True)
        env = dict(os.environ, LC_ALL="C")
        r = subprocess.run([sys.executable, os.path.join(HERE, "pe_rtti.py"),
                            os.path.abspath(dll_path)],
                           cwd=td, env=env, capture_output=True)
        p = os.path.join(td, "recon", "rtti.json")
        if os.path.exists(p):
            return json.load(open(p))
        sys.stderr.write("pe_rtti.py failed: " + r.stderr.decode()[:300] + "\n")
        return {}


def extract_rtti_names(pe, rtti_json):
    """Return (slot_va -> [name], ref_va -> [name]).

    * vtable slots are direct function pointers, so slot_va is a function VA;
    * ref_va addresses (vtable site / COL / TypeDescriptor) name the ctor/dtor
      or typeid sites that reference them.
    """
    tds = {int(k, 16): v for k, v in rtti_json.get("type_descriptors", {}).items()}
    cols = {int(k, 16): v for k, v in rtti_json.get("cols", {}).items()}
    vt = {int(k, 16): v for k, v in rtti_json.get("vtables", {}).items()}
    tsec = pe.section(".text")
    lo = pe.image_base + tsec["va"]
    hi = lo + tsec["vsize"]
    slot_names = defaultdict(set)
    ref_names = defaultdict(set)
    for col, sites in vt.items():
        c = cols.get(col)
        tdname = tds.get(c["td"]) if c else None
        if not tdname:
            continue
        for site in sites:
            q = site + 8
            i = 0
            while True:
                raw = pe.read_at_rva(q, 8)
                if len(raw) < 8:
                    break
                v = struct.unpack("<Q", raw)[0]
                if not (lo <= v < hi):
                    break
                slot_names[v].add(f"{tdname}::vfunc[{i}]")
                q += 8
                i += 1
            ref_names[pe.image_base + site].add(f"{tdname}::vtable[-1]")
            ref_names[pe.image_base + site + 8].add(f"{tdname}::vtable[0]")
            ref_names[pe.image_base + col].add(f"{tdname}::col")
        if c:
            ref_names[pe.image_base + c["td"]].add(f"{tdname}::typeinfo")
    return slot_names, ref_names


def build_side(dll, parity):
    pe = parity.PE(dll)
    asm = parity.run_objdump(dll)
    insns = parity.parse_objdump(asm, pe.image_base)
    funcs, meta = parity.build_functions(pe, insns)
    for f in funcs:
        f["refs"] = set()
    parity.prepare(funcs, pe.image_base, pe.size_of_image)
    return pe, funcs, meta


def recover_names(pe, funcs, rtti_json, extra=None):
    """Return (addr -> {name}, name -> {addr}, kind_by_name)."""
    funcsig_at = extract_funcsigs(pe)
    slot_names, ref_names = extract_rtti_names(pe, rtti_json)
    starts = [f["addr"] for f in funcs]

    def owner(va):
        i = bisect.bisect_right(starts, va) - 1
        if i >= 0 and funcs[i]["addr"] <= va < funcs[i]["end"]:
            return funcs[i]["addr"]
        if i + 1 < len(funcs) and funcs[i + 1]["addr"] == va:
            return funcs[i + 1]["addr"]
        return None

    addr_to_names = defaultdict(set)
    kind = {}

    def add(addr, name, k):
        if not addr or not name:
            return
        addr_to_names[addr].add(name)
        kind.setdefault(name, k)

    for f in funcs:
        for r in f["refs"]:
            nm = funcsig_at.get(r)
            if nm:
                add(f["addr"], nm, "funcsig")
            for nm in ref_names.get(r, ()):
                add(f["addr"], nm, "rtti-ref")
    for va, names in slot_names.items():
        o = owner(va)
        for nm in names:
            add(o, nm, "rtti-vfunc")
    if extra:
        for addr, names in extra.items():
            for nm in names:
                add(addr, nm, "funcsig-recon")

    name_to_addrs = defaultdict(set)
    for a, ns in addr_to_names.items():
        for n in ns:
            name_to_addrs[n].add(a)
    return addr_to_names, name_to_addrs, kind


def load_recon_names(pe):
    """Optional: canonicalise recon/functions.json (52 names) as a supplement."""
    out = defaultdict(set)
    p = "recon/functions.json"
    if not os.path.exists(p):
        return out
    try:
        raw = json.load(open(p))
    except Exception:
        return out
    for k, v in raw.items():
        try:
            addr = int(k, 16)
        except ValueError:
            continue
        for s in (v if isinstance(v, list) else [v]):
            nm = canonical_funcsig_name(s)
            if nm:
                out[addr].add(nm)
    return out


# ---------------------------------------------------------------------------
# pairing + divergence
# ---------------------------------------------------------------------------

def pair_by_name(no, nn):
    matched, ambiguous, only_o, only_n = [], [], [], []
    for nm in sorted(set(no) | set(nn)):
        ao, an = no.get(nm, set()), nn.get(nm, set())
        if ao and an:
            if len(ao) == 1 and len(an) == 1:
                matched.append((nm, next(iter(ao)), next(iter(an))))
            else:
                ambiguous.append((nm, sorted(ao), sorted(an)))
        elif ao:
            only_o.append((nm, sorted(ao)))
        else:
            only_n.append((nm, sorted(an)))
    return matched, ambiguous, only_o, only_n


def first_diff(a, b):
    for i in range(min(len(a), len(b))):
        if a[i] != b[i]:
            return i, a[i], b[i]
    if len(a) != len(b):
        i = min(len(a), len(b))
        return i, (a[i] if i < len(a) else "<end>"), (b[i] if i < len(b) else "<end>")
    return None


def compact_unified(a, b, n=2, max_lines=28):
    lines = list(unified_diff(a, b, fromfile="orig", tofile="new",
                              lineterm="", n=n))
    if len(lines) > max_lines:
        lines = lines[:max_lines] + [f"... (+{len(lines)-max_lines} more diff lines)"]
    return lines


def ratio_for(fa, fb):
    if fa["tokens"] == fb["tokens"]:
        return 1.0, True
    return SequenceMatcher(None, fa["tokens"], fb["tokens"],
                           autojunk=False).ratio(), False


# ---------------------------------------------------------------------------
# report
# ---------------------------------------------------------------------------

PLUGIN_HINT = ("Papyrus", "Catalog", "SceneState", "UiBridge", "FocusRecovery",
               "InputSink", "MenuVisibilitySink", "HandleFocusHotkey",
               "CreateViews", "SetCollapsed", "ConfirmModal", "Controller",
               "Config")

# Manual alias table: our v2 source name -> the original's recovered name for the
# SAME Papyrus native.  The rebuild renamed the natives (`Papyrus_Log` became
# `Papyrus::Natives::Log`, `Papyrus_CatalogBegin` became `Catalog::Begin`, ...),
# so exact-name pairing cannot link them.  The equivalences come from
# recon/NATIVES-RECOVERED.md and recon/PAPYRUS-CONTRACT.md (same Papyrus
# registration order and argument contract).  These are the actual parity
# targets of the plugin-owned tier.
KNOWN_ALIASES = {
    "Papyrus::Natives::Log": "`anonymous-namespace'::Papyrus_Log",
    "SceneState::BeginSceneSession": "`anonymous-namespace'::Papyrus_BeginSceneSession",
    "Catalog::Begin": "`anonymous-namespace'::Papyrus_CatalogBegin",
    "Catalog::Package": "`anonymous-namespace'::Papyrus_CatalogPackage",
    "Catalog::Finish": "`anonymous-namespace'::Papyrus_CatalogFinish",
    "Catalog::Publish": "`anonymous-namespace'::Papyrus_CatalogPublish",
    "SceneState::Publish": "`anonymous-namespace'::Papyrus_PublishSceneState",
    "SceneState::PublishCompatible": "`anonymous-namespace'::Papyrus_PublishCompatible",
    "SceneState::SetSearchQuery": "`anonymous-namespace'::Papyrus_SetSearchQuery",
}


def is_pluginish(name):
    return any(h in name for h in PLUGIN_HINT) or name.startswith("SKSEPlugin")


def plugin_tier_addresses(path, side="orig"):
    """Reuse the classification already written by tools/parity.py."""
    if not os.path.exists(path):
        return set()
    try:
        d = json.load(open(path))
    except Exception:
        return set()
    key = "orig_functions" if side == "orig" else "new_functions"
    return {int(r["addr"], 16) for r in d.get(key, []) if r.get("tier") == "plugin"}


def _divergence_rec(fa, fb):
    fd = first_diff(fa["tokens"], fb["tokens"])
    rec = {"first_diff": None,
           "diff": compact_unified(fa["tokens"], fb["tokens"], n=2, max_lines=12)}
    if fd:
        i, x, y = fd
        rec["first_diff"] = {"index": i, "orig": x, "new": y}
    return rec


def _emit_divergence(A, nm, oa, na, v, ratio, oi, ni, named):
    fa, fb = named["fby_o"][oa], named["fby_n"][na]
    A(f"#### `{nm}` - {v} ratio={ratio:.3f} - {oi}/{ni} insn "
      f"(orig 0x{oa:x} / new 0x{na:x})")
    A("")
    fd = first_diff(fa["tokens"], fb["tokens"])
    if fd:
        i, x, y = fd
        A(f"first differing normalised instruction #{i}:")
        A("")
        A("```")
        A(f"  orig: {x}")
        A(f"  new : {y}")
        A("```")
    A("")
    A("```diff")
    for line in compact_unified(fa["tokens"], fb["tokens"]):
        A(line)
    A("```")
    A("")


def write_report(path, args, meta, named, matched, ambiguous, only_o, only_n,
                 plugin_addrs, plugin_named, plugin_paired, tier_note,
                 alias_unresolved=()):
    L = []
    A = L.append
    A("# Name-Aware Disassembly Parity - SexLabPPrism.dll vs rebuild v2")
    A("")
    A("Generated by `tools/parity_names.py` (machinery imported from "
      "`tools/parity.py`). Both DLLs read-only.")
    A("")
    A("This report pairs functions **by name**, recovering names from embedded")
    A("`__FUNCSIG__` literals and MSVC RTTI on both sides. It is the actionable")
    A("counterpart to the name-blind structural pairing in `recon/parity-v2.md`:")
    A("a function that compiled differently but is the same source function is")
    A("reported here as a named pair with a ratio and a diff, instead of")
    A("`MISSING-NEW`.")
    A("")

    A("## 0. Recovery summary")
    A("")
    A("| side | functions | funcsigs | funcsig-named fns | RTTI type descr | RTTI vtable slots | names total |")
    A("|---|---|---|---|---|---|---|")
    for tag in ("orig", "new"):
        m = named[tag]
        A(f"| {tag} | {m['nfunc']} | {m['nsig']} | {m['nsig_named']} | "
          f"{m['ntypes']} | {m['nslots']} | {m['nnames']} |")
    A("")
    A("`RTTI vtable slots` is the number of virtual-function pointers found in")
    A("class vtables (each also names a real function address).")
    A("")

    A("## 1. Named-pair verdicts")
    A("")
    vc = Counter(v for _, _, r, v, _, _, _ in matched)
    A("| verdict | pairs |")
    A("|---|---|")
    for v in ("EXACT", "CLOSE", "SIMILAR", "DIFFERENT"):
        A(f"| {v} | {vc.get(v,0)} |")
    A(f"| **total** | **{len(matched)}** |")
    A("")
    distinct = len({(r[1], r[2]) for r in matched})
    A(f"- names present on both sides (unambiguous): **{len(matched)}**")
    A(f"- distinct original-to-rebuild function-address pairs: **{distinct}**")
    A("  (one function may carry several RTTI vfunc names, so name rows exceed")
    A("  distinct function pairs)")
    A(f"- names ambiguous on either side: **{len(ambiguous)}**")
    A(f"- names on the original only: **{len(only_o)}**")
    A(f"- names on the rebuild only: **{len(only_n)}**")
    A("")
    A("A name is *ambiguous* when it resolves to !=1 function on either side")
    A("(duplicate literals, ICF-folded copies, multi-vtable slots). Those are")
    A("excluded from the pair table below and listed in section 4.")
    A("")

    # ---- full table ----
    A("## 2. Named-pair table (all recoverable names)")
    A("")
    A("Original name -> verdict -> ratio -> orig/new instruction counts.")
    A("`kind`: funcsig = `__FUNCSIG__`, rtti-vfunc = vtable slot,")
    A("rtti-ref = ctor/dtor/typeid site.  `*` marks plugin-relevant names.")
    A("")
    A("| orig name | kind | verdict | ratio | orig addr | new addr | orig insn | new insn |")
    A("|---|---|---|---|---|---|---|---|")
    for nm, oa, na, v, ratio, oi, ni in matched:
        mark = "*" if is_pluginish(nm) else ""
        A(f"| {mark}{nm[:78].replace('|','/')} | {named['kind'].get(nm,'?')} | {v} | "
          f"{ratio:.3f} | 0x{oa:x} | 0x{na:x} | {oi} | {ni} |")
    A("")
    A("### 2b. Manually aliased pairs (renamed Papyrus natives)")
    A("")
    A("The rebuild renamed the natives, so these are linked by the explicit")
    A("`KNOWN_ALIASES` table in `tools/parity_names.py` rather than by string")
    A("equality. These are the genuine plugin-tier parity targets.")
    A("")
    A("| orig name | our v2 name | verdict | ratio | orig addr | new addr | orig insn | new insn |")
    A("|---|---|---|---|---|---|---|---|")
    our_of = {v: k for k, v in KNOWN_ALIASES.items()}
    for nm, oa, na, v, ratio, oi, ni in matched:
        if named['kind'].get(nm) == "alias":
            A(f"| {nm[:60]} | {our_of.get(nm,'?')} | {v} | {ratio:.3f} | "
              f"0x{oa:x} | 0x{na:x} | {oi} | {ni} |")
    A("")
    if alias_unresolved:
        A("Unresolved aliases (name missing or ambiguous on a side):")
        A("")
        A("| our v2 name | intended orig name | orig addrs | new addrs |")
        A("|---|---|---|---|")
        for our, orig, na, nn_ in alias_unresolved:
            A(f"| {our} | {orig[:60]} | {na} | {nn_} |")
        A("")

    # ---- plugin coverage ----
    A("## 3. Plugin-owned coverage (the 399-problem)")
    A("")
    A(f"- original plugin-tier functions: **{len(plugin_addrs)}**")
    A(f"- of those, with ANY recovered name: **{len(plugin_named)}**")
    A(f"- of those, with a NAME-PAIRED rebuild counterpart: **{len(plugin_paired)}**")
    A("")
    A("A named counterpart here means the name resolves to exactly one function on")
    A("both sides, so the disassembly ratio below is meaningful.  This is the")
    A("number that the structural harness's 228 `MISSING-NEW` understated.")
    A("")
    if tier_note:
        A(tier_note)
        A("")
    A("### Original plugin functions with no named counterpart")
    A("")
    unnamed = sorted(plugin_addrs - plugin_paired)
    A(f"{len(unnamed)} functions; first 60:")
    A("")
    A("| orig addr | names recovered |")
    A("|---|---|")
    for a in unnamed[:60]:
        ns = ", ".join(sorted(named['addr_to_names'].get(a, ())))[:80]
        A(f"| 0x{a:x} | {ns or '-'} |")
    A("")

    # ---- one-sided ----
    A("## 4. Names present on one side only")
    A("")
    A("### 4a. Original-only (the parity targets we have not named-matched)")
    A("")
    A("| name | kind | orig addr(s) | orig insn |")
    A("|---|---|---|---|")
    o_by = named["fby_o"]
    for nm, addrs in only_o:
        mark = "*" if is_pluginish(nm) else ""
        ins = ",".join(str(o_by[a]["icount"]) for a in addrs if a in o_by)
        A(f"| {mark}{nm[:80].replace('|','/')} | {named['kind'].get(nm,'?')} | "
          f"{','.join(hex(a) for a in addrs)} | {ins} |")
    A("")
    A("### 4b. Rebuild-only (OUR new source layout)")
    A("")
    A("These are functions we authored that have no same-named counterpart in the")
    A("original.  Their original counterparts must be identified manually; the")
    A("`*` names are the plugin surface we care about.")
    A("")
    A("| name | kind | new addr(s) | new insn |")
    A("|---|---|---|---|")
    n_by = named["fby_n"]
    for nm, addrs in only_n:
        mark = "*" if is_pluginish(nm) else ""
        ins = ",".join(str(n_by[a]["icount"]) for a in addrs if a in n_by)
        A(f"| {mark}{nm[:80].replace('|','/')} | {named['kind'].get(nm,'?')} | "
          f"{','.join(hex(a) for a in addrs)} | {ins} |")
    A("")

    # ---- ambiguous ----
    A("## 5. Ambiguous names (excluded from pairing)")
    A("")
    A("| name | kind | orig addrs | new addrs |")
    A("|---|---|---|---|")
    for nm, ao, an in ambiguous:
        A(f"| {nm[:80].replace('|','/')} | {named['kind'].get(nm,'?')} | "
          f"{','.join(hex(a) for a in ao) or '-'} | {','.join(hex(a) for a in an) or '-'} |")
    A("")

    # ---- convergence worklist ----
    A("## 6. Convergence worklist")
    A("")
    A("Ranked by ratio. `in reach` (0.6-0.9) entries carry the first differing")
    A("normalised instruction and a compact unified diff - that is what to change.")
    A("")
    inreach = [m for m in matched if 0.6 <= m[4] < 0.9]
    close = [m for m in matched if 0.9 <= m[4] < 1.0]
    hopeless = [m for m in matched if m[4] < 0.6]
    A(f"- closest to parity (ratio 0.9-1.0): **{len(close)}**")
    A(f"- in reach (0.6-0.9): **{len(inreach)}**")
    A(f"- below 0.6 (different toolchain / inlining / not yet ported): "
      f"**{len(hopeless)}**")
    A("")
    A("### 6a. Closest to parity (0.9-1.0)")
    A("")
    A("| ratio | verdict | name | orig/new insn |")
    A("|---|---|---|---|")
    for nm, oa, na, v, ratio, oi, ni in sorted(close, key=lambda x: -x[4])[:80]:
        A(f"| {ratio:.3f} | {v} | {nm[:78].replace('|','/')} | {oi}/{ni} |")
    A("")
    A("### 6b. In reach (0.6-0.9): what differs")
    A("")
    for nm, oa, na, v, ratio, oi, ni in sorted(inreach, key=lambda x: -x[4]):
        _emit_divergence(A, nm, oa, na, v, ratio, oi, ni, named)
    A("### 6b2. Every plugin / aliased / `__FUNCSIG__` pair: what differs")
    A("")
    A("This is the actionable core: the original plugin surface and the"
      "reconstructed `__FUNCSIG__`-named functions, whatever their ratio.")
    A("")
    siglike = [m for m in matched
               if m not in inreach and
               (named['kind'].get(m[0]) in ("alias", "funcsig", "funcsig-recon")
                or is_pluginish(m[0]))]
    for nm, oa, na, v, ratio, oi, ni in sorted(siglike, key=lambda x: -x[4]):
        _emit_divergence(A, nm, oa, na, v, ratio, oi, ni, named)
    A("### 6c. Below 0.6 (hopeless without the original toolchain, unless re-ported)")
    A("")
    A("| ratio | name | orig/new insn | note |")
    A("|---|---|---|---|")
    for nm, oa, na, v, ratio, oi, ni in sorted(hopeless, key=lambda x: -x[4])[:80]:
        if ni and max(oi, ni) / max(min(oi, ni), 1) > 2.0:
            note = "instruction-count blow-up (inlining / different algorithm)"
        else:
            note = "different code shape / register allocation"
        A(f"| {ratio:.3f} | {nm[:78].replace('|','/')} | {oi}/{ni} | {note} |")
    A("")

    # ---- limits ----
    A("## 7. What the name-aware metric still cannot see")
    A("")
    A("- **Unnamed functions are invisible to it.** A function only appears here")
    A("  if some literal or vtable slot resolves to its address.  The original's")
    A("  private leaf helpers without a log site and without RTTI stay unnamed;")
    A("  they are still only in the structural pair set.")
    A("- **Names are recovered, not debug symbols.** `__FUNCSIG__` gives the")
    A("  signature of the *logged* function; a function that logs another")
    A("  function's signature would be mis-named (rare but possible).")
    A("- **Renamed reconstructions do not auto-pair.** Our `Papyrus::Natives::Log`")
    A("  and the original's `Papyrus_Log` are different name keys, so they appear")
    A("  in 4a/4b, not in the pair table. Pairing them needs a manual alias table.")
    A("- **RTTI vtable-slot names can be ambiguous.** ICF folding, thunks and")
    A("  multiple vtables pointing at one address make a name resolve to several")
    A("  functions; those are excluded (section 5).")
    A("- **Same name != same semantics.** Name equality only asserts the same")
    A("  source signature; the ratio/diff is still a token-order heuristic, not a")
    A("  proof of correctness.")
    A("- **Inlining moves code across name boundaries.** A named pair can have a")
    A("  low ratio simply because the compiler inlined a callee into one side.")
    A("")
    A("## 8. Reproduce")
    A("")
    A("```")
    A(f"python3 tools/parity_names.py --orig {args.orig} --new {args.new} "
      f"--report {args.report}")
    A("python3 tools/parity.py       # structural baseline (self-test 18/18)")
    A("```")
    open(path, "w").write("\n".join(L) + "\n")


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--orig", default="artifacts/SexLabPPrism.dll")
    ap.add_argument("--new", default="artifacts/rebuild/SexLabPPrism-rebuild-v2.dll")
    ap.add_argument("--report", default="recon/parity-names-v2.md")
    ap.add_argument("--outdir", default="recon/parity-names-v2")
    ap.add_argument("--per-function", default="recon/parity-v2/per-function.json")
    args = ap.parse_args()

    os.makedirs(args.outdir, exist_ok=True)

    pe_o, fo, meta_o = build_side(args.orig, P)
    pe_n, fn_, meta_n = build_side(args.new, P)

    rtti_o = recover_rtti(args.orig)
    rtti_n = recover_rtti(args.new)

    extra_o = load_recon_names(pe_o)
    ao, no, kind_o = recover_names(pe_o, fo, rtti_o, extra_o)
    an, nn, kind_n = recover_names(pe_n, fn_, rtti_n, None)
    kind = dict(kind_n)
    kind.update(kind_o)

    named = {
        "nfunc": len(fo), "nsig": len(extract_funcsigs(pe_o)),
        "nsig_named": sum(1 for a, ns in ao.items()
                          if any(kind.get(n) in ("funcsig", "funcsig-recon") for n in ns)),
        "ntypes": rtti_o.get("count_types", 0),
        "nslots": sum(len(v) for v in extract_rtti_names(pe_o, rtti_o)[0].values()),
        "nnames": len(no), "kind": kind,
        "addr_to_names": ao, "fby_o": {f["addr"]: f for f in fo}, "fby_n": {f["addr"]: f for f in fn_},
    }
    named_n = {
        "nfunc": len(fn_), "nsig": len(extract_funcsigs(pe_n)),
        "nsig_named": sum(1 for a, ns in an.items()
                          if any(kind.get(n) in ("funcsig", "funcsig-recon") for n in ns)),
        "ntypes": rtti_n.get("count_types", 0),
        "nslots": sum(len(v) for v in extract_rtti_names(pe_n, rtti_n)[0].values()),
        "nnames": len(nn),
    }
    named["orig"] = {k: named[k] for k in
                     ("nfunc", "nsig", "nsig_named", "ntypes", "nslots", "nnames")}
    named["new"] = named_n

    fby_o = named["fby_o"]
    fby_n = named["fby_n"]
    matched, ambiguous, only_o, only_n = pair_by_name(no, nn)

    # attach ratio / verdict / counts
    pair_rows = []
    for nm, oa, na in matched:
        fa, fb = fby_o.get(oa), fby_n.get(na)
        if not fa or not fb:
            continue
        ratio, exact = ratio_for(fa, fb)
        v = P.EXACT if exact else P.verdict_for(ratio)
        pair_rows.append((nm, oa, na, v, ratio, fa["icount"], fb["icount"]))

    # manual alias pairs for the renamed Papyrus natives
    alias_unresolved = []
    alias_o, alias_n = set(), set()
    for our, orig in KNOWN_ALIASES.items():
        ao_, an_ = no.get(orig), nn.get(our)
        if ao_ and an_ and len(ao_) == 1 and len(an_) == 1:
            oa, na = next(iter(ao_)), next(iter(an_))
            fa, fb = fby_o[oa], fby_n[na]
            ratio, exact = ratio_for(fa, fb)
            v = P.EXACT if exact else P.verdict_for(ratio)
            pair_rows.append((orig, oa, na, v, ratio, fa["icount"], fb["icount"]))
            kind[orig] = "alias"
            alias_o.add(orig)
            alias_n.add(our)
        else:
            alias_unresolved.append((our, orig, len(ao_ or ()), len(an_ or ())))
    # move alias-paired names out of the one-sided / ambiguous lists
    only_o = [(n, a) for n, a in only_o if n not in alias_o]
    only_n = [(n, a) for n, a in only_n if n not in alias_n]
    ambiguous = [(n, ao, an) for n, ao, an in ambiguous if n not in alias_o]
    pair_rows.sort(key=lambda r: -r[4])

    # plugin coverage
    plugin_addrs = plugin_tier_addresses(args.per_function, "orig")
    matched_names = {r[0] for r in pair_rows}
    plugin_named = {a for a in plugin_addrs if ao.get(a)}
    plugin_paired = {a for a in plugin_addrs
                     if any(n in matched_names for n in ao.get(a, ()))}
    tier_note = ""
    if not plugin_addrs:
        tier_note = ("(`recon/parity-v2/per-function.json` missing; run "
                     "`python3 tools/parity.py ... --outdir recon/parity-v2` first.)")

    write_report(args.report, args, {"o": meta_o, "n": meta_n}, named, pair_rows,
                 ambiguous, only_o, only_n, plugin_addrs, plugin_named,
                 plugin_paired, tier_note, alias_unresolved)

    json.dump({
        "orig": {"funcs": len(fo), "funcsigs": named["nsig"], "types": named["ntypes"],
                 "names": len(no)},
        "new": {"funcs": len(fn_), "funcsigs": named_n["nsig"], "types": named_n["ntypes"],
                "names": len(nn)},
        "matched": [dict({"name": nm, "orig": hex(oa), "new": hex(na),
                          "verdict": v, "ratio": round(r, 6),
                          "orig_insn": oi, "new_insn": ni},
                         **_divergence_rec(fby_o[oa], fby_n[na]))
                    for nm, oa, na, v, r, oi, ni in pair_rows],
        "ambiguous": [{"name": nm, "orig": [hex(a) for a in ao], "new": [hex(a) for a in an]}
                      for nm, ao, an in ambiguous],
        "only_orig": [{"name": nm, "orig": [hex(a) for a in addrs]} for nm, addrs in only_o],
        "only_new": [{"name": nm, "new": [hex(a) for a in addrs]} for nm, addrs in only_n],
        "plugin": {"total": len(plugin_addrs), "named": len(plugin_named),
                   "paired": len(plugin_paired)},
    }, open(os.path.join(args.outdir, "names.json"), "w"), indent=1)

    # ---- compact stdout (< 40 lines) ----
    vc = Counter(r[3] for r in pair_rows)
    print(f"functions: orig {len(fo)}, new {len(fn_)}")
    print(f"funcsig names: orig {named['nsig']} literals / {named['nsig_named']} fns, "
          f"new {named_n['nsig']} literals / {named_n['nsig_named']} fns")
    print(f"RTTI: orig {named['ntypes']} types / {named['nslots']} vtable slots, "
          f"new {named_n['ntypes']} types / {named_n['nslots']} vtable slots")
    print(f"unique names: orig {len(no)}, new {len(nn)}")
    distinct = len({(r[1], r[2]) for r in pair_rows})
    print(f"name pairs: {len(pair_rows)} names / {distinct} distinct function pairs  " +
          " ".join(f"{k}={vc.get(k,0)}" for k in (P.EXACT, P.CLOSE, P.SIMILAR, P.DIFFERENT)))
    print(f"ambiguous names: {len(ambiguous)}, orig-only: {len(only_o)}, new-only: {len(only_n)}")
    if plugin_addrs:
        print(f"plugin-owned orig: {len(plugin_addrs)}, named {len(plugin_named)}, "
              f"name-paired {len(plugin_paired)}")
    cand = [r for r in pair_rows if r[4] < 1.0]
    pref = [r for r in cand
            if kind.get(r[0]) in ("alias", "funcsig", "funcsig-recon")
            or is_pluginish(r[0])]
    top = (pref or cand)[:10]
    print(f"top convergence targets ({len(pref)} plugin/aliased/__FUNCSIG__ names):")
    for nm, oa, na, v, r, oi, ni in top:
        print(f"  {r:.3f} {v:9s} {nm[:52]} ({oi}/{ni} insn)")
    print(f"wrote {args.report}")
    print(f"wrote {os.path.join(args.outdir, 'names.json')}")


if __name__ == "__main__":
    main()