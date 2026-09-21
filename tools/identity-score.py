#!/usr/bin/env python3
"""Append the honest before/after scoreboard to recon/identity-matching.md.

Reads:
  * recon/identity-matching.json          (anchors + pairs, tools/pair_identity.py)
  * recon/identity-before.json            (match.py structural, real denominator)
  * recon/matching/per-function.json      (match.py --identity, real denominator)
  * artifacts/SexLabPPrism.dll            (to recover the REAL .pdata function set)

Writes recon/identity-matching.md sections: before/after split on the 1,495 real
functions, the anchor table summary, and the closest-first convergence worklist
weighted by (1-ratio) * our instruction count.
"""
import collections
import json
import pathlib
import struct
import sys

W = pathlib.Path.home() / "Projects/SexLabPPrism-re"
ORDER = ["BYTE-MATCH", "INSN-MATCH", "RATIO", "MISSING"]


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


def split(entries, starts, base):
    rows = []
    for e in entries:
        a = e.get("orig_addr")
        va = int(a, 16) if isinstance(a, str) else a
        rva = va - base if va >= base else va
        if rva in starts:
            rows.append(e)
    return rows


def summarise(rows):
    c = collections.Counter(str(r["verdict"]).upper() for r in rows)
    insn = sum(int(r.get("orig_insn") or 0) for r in rows)
    out = {"n": len(rows), "insn": insn, "counts": c}
    out["byte_insn"] = sum(int(r.get("orig_insn") or 0) for r in rows
                           if str(r["verdict"]).upper() == "BYTE-MATCH")
    return out


def fmt(s, label):
    lines = [f"{label}: {s['n']:,} functions, {s['insn']:,} instructions"]
    for v in ORDER:
        c = s["counts"].get(v, 0)
        lines.append(f"- {v}: {c:,} ({c / s['n'] * 100:.1f}%)")
    lines.append(f"- byte-match: {s['byte_insn'] / s['insn'] * 100:.2f}% of instructions")
    return "\n".join(lines)


def main():
    before_path = sys.argv[1] if len(sys.argv) > 1 else "recon/identity-before.json"
    after_path = sys.argv[2] if len(sys.argv) > 2 else "recon/matching/per-function.json"
    report = sys.argv[3] if len(sys.argv) > 3 else "recon/identity-matching.md"

    data = (W / "artifacts/SexLabPPrism.dll").read_bytes()
    starts, base = pdata_starts(data)
    before = json.loads((W / before_path).read_text())
    after = json.loads((W / after_path).read_text())
    ident = json.loads((W / "recon/identity-matching.json").read_text())
    be = before if isinstance(before, list) else before.get("functions", [])
    ae = after if isinstance(after, list) else after.get("functions", [])
    sb = summarise(split(be, starts, base))
    sa = summarise(split(ae, starts, base))

    L = []
    A = L.append
    A("")
    A("## Before/after on the REAL-function denominator (tools/real-functions-score.py)")
    A("")
    A(f"`{len(starts):,}` functions with an unwind entry. **Before** = structural")
    A("pairing (`tools/match.py --new ...-lto2.dll`, Round 1). **After** = identity")
    A("pairing (`tools/match.py --new ...-lto2.dll --identity`).")
    A("")
    A("| verdict | before | after | delta |")
    A("|---|---|---|---|")
    for v in ORDER:
        b, a = sb["counts"].get(v, 0), sa["counts"].get(v, 0)
        A(f"| {v} | {b:,} | {a:,} | {a - b:+,} |")
    A(f"| **functions** | **{sb['n']:,}** | **{sa['n']:,}** | |")
    A(f"| byte-match, instruction-weighted | {sb['byte_insn']/sb['insn']*100:.2f}% "
      f"| {sa['byte_insn']/sa['insn']*100:.2f}% "
      f"| {(sa['byte_insn']/sa['insn']-sb['byte_insn']/sb['insn'])*100:+.2f} pp |")
    A("")
    A("**Headline change:** "
      f"MISSING {sb['counts'].get('MISSING',0):,} -> {sa['counts'].get('MISSING',0):,}; "
      f"BYTE-MATCH {sb['counts'].get('BYTE-MATCH',0):,} -> {sa['counts'].get('BYTE-MATCH',0):,}. "
      "The identity pairing mostly converts *spurious MISSING* into honest RATIO")
    A("(a true counterpart that has not converged yet), which is the whole point:")
    A("source progress is no longer invisible.")
    A("")
    A("## Anchor table summary")
    A("")
    A("| anchor | pairs | confidence |")
    A("|---|---|---|")
    ANCH = ident["anchors"]
    for k in ("name", "alias", "rtti", "string", "callgraph", "icf", "structural"):
        conf = "LOW (excluded from headline)" if k == "structural" else "high"
        A(f"| {k} | {ANCH.get(k,0):,} | {conf} |")
    A(f"| **total** | **{sum(ANCH.values()):,}** | |")
    A("")
    ur = ident.get("alias_unresolved", [])
    if ur:
        A("Aliases that did NOT resolve on lto2 (no silent drop): "
          + "; ".join(f"`{u['our']}` (orig {'ok' if u['orig_found'] else '-'}/"
                      f"new {'ok' if u['new_found'] else '-'})" for u in ur) + ".")
        A("")

    A("## TRUE closest-first convergence worklist")
    A("")
    A("Real functions with a *paired* counterpart (any anchor) that is not yet")
    A("byte-equal. Ranked by weighted deficit = `(1 - ratio) * our_insn` (the")
    A("volume of OUR divergent code to reconcile), so the biggest, most faithful")
    A("pairs come first. `anchor` says why we trust the pair.")
    A("")
    rows = []
    for e in split(ae, starts, base):
        v = str(e["verdict"]).upper()
        if v in ("BYTE-MATCH",) or not e.get("new_addr"):
            continue
        r = float(e.get("ratio") or 0.0)
        oi = int(e.get("orig_insn") or 0)
        ni = int(e.get("our_insn") or oi)
        rows.append((r, (1 - r) * ni, e, oi, ni))
    rows.sort(key=lambda x: -x[1])
    A("")
    A("| # | weighted deficit | ratio | anchor | verdict | orig addr | new addr | orig insn | our insn | name |")
    A("|---|---|---|---|---|---|---|---|---|---|")
    for k, (r, d, e, oi, ni) in enumerate(rows[:30], 1):
        nm = (e.get("name") or "")[:52] or "-"
        A(f"| {k} | {d:.0f} | {r:.3f} | {e.get('anchor') or '-'} | {e['verdict']} | "
          f"{e['orig_addr']} | {e.get('new_addr')} | {oi} | {ni} | {nm} |")
    A("")

    with open(W / report, "a", encoding="utf-8") as fh:
        fh.write("\n".join(L) + "\n")
    print(f"appended {len(L)} lines to {report}")
    print(f"before {dict(sb['counts'])} -> after {dict(sa['counts'])}")


if __name__ == "__main__":
    main()