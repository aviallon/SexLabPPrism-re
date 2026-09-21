#!/usr/bin/env python3
"""Typed inventory of the spdlog/MSVC-STL types an image instantiates.

Read-only. Uses tools/pe_rtti.py (re-run first to refresh recon/) and the raw
string table. It answers the question "which spdlog formatters/sinks exist in
this DLL?", which is a *compile-time* property of the image, not of the runtime
pattern string.

Usage:
    python3 tools/pe_rtti.py <dll>            # writes recon/rtti.txt + rtti.json
    python3 tools/spdlog_inventory.py <rtti.txt> <dll>

Two images can then be compared with --diff.
"""
import re
import sys
from collections import Counter

FORMAT_RE = re.compile(r"ratio@\$00\$([0-9A-Z]+)@")


def type_descriptors(path):
    """Return {mangled_name: TD_rva_hex} from a pe_rtti.py JSON report.

    Prefer the .json over the .txt: rtti.txt truncates names at 70 chars, and
    the elapsed_formatter unit is only visible in the full name.
    """
    import json
    if path.endswith(".json"):
        d = json.load(open(path))
        return {v: k for k, v in d["type_descriptors"].items()}
    out = {}
    inside = False
    for ln in open(path, encoding="latin1"):
        if ln.startswith("== TYPE DESCRIPTORS"):
            inside = True
            continue
        if inside and ln.startswith("=="):
            break
        if not inside:
            continue
        m = re.match(r"^(\S+)\s+TD=0x([0-9a-f]+)$", ln.rstrip())
        if m:
            out[m.group(1)] = m.group(2)
    return out


def family(name):
    """Coarse family for a mangled spdlog/STL type descriptor."""
    if "?$elapsed_formatter@" in name:
        pad = "null" if "Unull_scoped_padder" in name else "scoped"
        m = FORMAT_RE.search(name)
        unit = m.group(1) if m else "?"
        return f"formatter/elapsed/{pad}/{unit}"
    if "formatter@" in name:
        m = re.search(r"\?([A-Za-z])_formatter@", name)
        base = m.group(1) if m else "?"
        pad = "null" if "Unull_scoped_padder" in name else (
            "scoped" if "Vscoped_padder" in name else "none")
        return f"formatter/{base}/{pad}"
    if "@sinks@spdlog@@" in name or "@spdlog@@@" in name and "_sink" in name:
        return "sink/" + name.split("?$")[-1].split("@")[0]
    if "pattern_formatter" in name:
        return "pattern_formatter"
    if "logger@spdlog@@" in name:
        return "logger"
    if "spdlog_ex" in name:
        return "spdlog_ex"
    if "detail" in name or "@std@@" in name:
        return "stl/other"
    return "other"


def inventory(rtti_path):
    tds = type_descriptors(rtti_path)
    spd = {k: v for k, v in tds.items()
           if "spdlog" in k or "_formatter" in k}
    fam = Counter(family(k) for k in spd)
    return spd, fam


def patterns(dll_path):
    """Every printf/fmt-looking literal that could be an spdlog pattern."""
    d = open(dll_path, "rb").read()
    seen = set()
    for m in re.finditer(rb"[\x20-\x7e]{4,}", d):
        s = m.group().decode("latin1")
        if s.startswith("[%") or re.match(r"^\[%[YmdHMSelvTtpPonsqbcCrRxXEZz]", s):
            seen.add(s)
        if re.search(r"%[-#0-9.+]*[YmdHMSelvTt]", s) and "[" in s:
            seen.add(s)
    return sorted(seen)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    a = sys.argv[1]
    if a == "--diff":
        so, _ = inventory(sys.argv[2])
        sp, _ = inventory(sys.argv[3])
        only_o = sorted(set(so) - set(sp))
        only_p = sorted(set(sp) - set(so))
        print("spdlog TDs: A=%d B=%d  only-A=%d only-B=%d"
              % (len(so), len(sp), len(only_o), len(only_p)))
        for k in only_o:
            print("  ONLY-A", k)
        for k in only_p:
            print("  ONLY-B", k)
        return 0
    spd, fam = inventory(a)
    print("== %s ==" % a)
    print("spdlog type descriptors: %d" % len(spd))
    for f, n in sorted(fam.items()):
        print("  %-40s %d" % (f, n))
    if len(sys.argv) > 2:
        print("pattern-like literals:")
        for s in patterns(sys.argv[2]):
            print("  %r" % s)
    return 0


if __name__ == "__main__":
    sys.exit(main())