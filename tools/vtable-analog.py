#!/usr/bin/env python3
"""Map original library functions to rebuild counterparts through MSVC RTTI
vtables, to separate H3 (different instantiation / shape) from H4 (absent).

For an original function that sits in vtable slot k of class-T's vtable, find
the rebuild's vtable for the same type descriptor T and read slot k.  That is
the same virtual method, compiled from the same source by the same compiler,
so a large structural difference there is shape, and its complete absence from
every vtable is closer to H4.
"""
from __future__ import annotations

import json
import os
import struct
import sys

REPO = "/home/aviallon/Projects/SexLabPPrism-re"
sys.path.insert(0, os.path.join(REPO, "tools"))
import parity  # noqa: E402

# Paths are overridable so the tool can be pointed at the current parity build
# instead of the round-5 DLL it was originally written against:
#   --orig PATH --new PATH --rtti-new PATH --rtti-orig PATH
# (previously the new side was hard-wired to parity-r5.dll + /tmp/rttinew).
def _opt(flag, default):
    if flag in sys.argv:
        return sys.argv[sys.argv.index(flag) + 1]
    return default


def _drop(*flags):
    out, i = [], 0
    while i < len(sys.argv):
        if sys.argv[i] in flags:
            del sys.argv[i:i + 2]
            continue
        out.append(sys.argv[i])
        i += 1
    return out


ORIG = _opt("--orig", os.path.join(REPO, "artifacts/SexLabPPrism.dll"))
NEW = _opt("--new", os.path.join(REPO, "artifacts/rebuild/parity-r6.dll"))
RTTI_O = _opt("--rtti-orig", "/tmp/rttio/recon/rtti.json")
RTTI_N = _opt("--rtti-new", "/tmp/rtti6/recon/rtti.json")
sys.argv = _drop("--orig", "--new", "--rtti-orig", "--rtti-new")


def read_slots(pe, rtti):
    """td_name -> {slot_index: func_rva}.  A COL's vtable site points at the
    COL; the vtable itself (slot 0) starts 8 bytes later."""
    out = {}
    tds = rtti["type_descriptors"]
    for col, sites in rtti["vtables"].items():
        td_rva = rtti["cols"][col]["td"]
        td = tds.get(hex(td_rva), "?")
        for s in sites:
            va = pe.image_base + s + 8
            sl = 0
            while sl < 256:
                try:
                    q = struct.unpack_from("<Q", pe.data, pe.rva2off(va - pe.image_base))[0]
                except Exception:
                    break
                rva = q - pe.image_base
                if not (pe.image_base < q < pe.image_base + pe.size_of_image):
                    break
                out.setdefault(td, {}).setdefault(sl, rva)
                va += 8
                sl += 1
    return out


def func_insns(perfunc):
    return perfunc


def main():
    pe_o = parity.PE(ORIG)
    pe_n = parity.PE(NEW)
    def _rtti(path, dll):
        if os.path.exists(path):
            return json.load(open(path))
        # fall back to recovering RTTI on the fly
        sys.path.insert(0, os.path.join(REPO, "tools"))
        import parity_names  # noqa: E402
        return parity_names.recover_rtti(dll)

    ro = _rtti(RTTI_O, ORIG)
    rn = _rtti(RTTI_N, NEW)
    so = read_slots(pe_o, ro)
    sn = read_slots(pe_n, rn)
    pf = json.load(open(os.path.join(REPO, "recon/matching/per-function.json")))
    verdict = {int(r["orig_addr"], 16): r for r in pf["functions"]}
    target = sys.argv[1:] if len(sys.argv) > 1 else None

    # orig function rva -> (td, slot)
    inv = {}
    for td, slots in so.items():
        for slot, rva in slots.items():
            inv.setdefault(pe_o.image_base + rva, []).append((td, slot))

    rows = []
    for r in pf["functions"]:
        if r["tier"] != "library" or r["verdict"] != "MISSING":
            continue
        if target and r["orig_addr"] not in target:
            continue
        va = int(r["orig_addr"], 16)
        for td, slot in inv.get(va, [(None, None)]):
            nslot = sn.get(td, {}).get(slot) if td else None
            rows.append((r["orig_insn"], r["orig_addr"], td, slot, nslot))
    rows.sort(key=lambda x: -x[0])
    net = 0
    for insn, addr, td, slot, nslot in rows[:80]:
        print(f"{addr} insn={insn:4d} slot={slot} td={str(td)[:52]:52s} newslot_va="
              f"{hex(pe_n.image_base + nslot) if nslot else None}")
    # arithmetic over all 660
    tot = sum(1 for r in pf["functions"] if r["tier"] == "library" and r["verdict"] == "MISSING")
    invd = sum(1 for r in pf["functions"] if r["tier"] == "library" and r["verdict"] == "MISSING"
               and inv.get(int(r["orig_addr"], 16)))
    print(f"library MISSING {tot}, of which vtable-located {invd}")


if __name__ == "__main__":
    main()
