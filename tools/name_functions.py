#!/usr/bin/env python3
"""Map __FUNCSIG__ string literals embedded in the binary back to the
function that references them, using .pdata function bounds.

The MSVC compiler emits the enclosing function's __FUNCSIG__ as the
spdlog source_location function name at every log call site, so each
unique signature string is referenced by exactly the function(s) that
logged. This recovers real C++ function names for otherwise anonymous code.
"""
import struct, re, subprocess, sys, json, os

PATH = sys.argv[1] if len(sys.argv) > 1 else "artifacts/SexLabPPrism.dll"
data = open(PATH, "rb").read()
IB = 0x180000000
e = struct.unpack_from("<I", data, 0x3C)[0]
coff = e + 4
nsec, = struct.unpack_from("<H", data, coff + 2)
optsz, = struct.unpack_from("<H", data, coff + 16)
opt = coff + 20
sect = opt + optsz
secs = []
for i in range(nsec):
    o = sect + i * 40
    nm = data[o:o+8].rstrip(b"\0").decode()
    vs, va, rs, raw = struct.unpack_from("<IIII", data, o + 8)
    secs.append((nm, va, vs, raw, rs))

def off2rva(off):
    for nm, va, vs, raw, rs in secs:
        if raw <= off < raw + rs:
            return va + (off - raw)

# .pdata
pdr, pds = struct.unpack_from("<II", data, opt + 112 + 24)
po = [x for x in secs if x[0] == ".pdata"][0][3]
funcs = []
n = pds // 12
for i in range(n):
    b, en, uw = struct.unpack_from("<III", data, po + i * 12)
    funcs.append((b, en, uw))
funcs.sort()
starts = [f[0] for f in funcs]

def owner(insn_rva):
    # linear scan is fine for 1495
    for b, en, uw in funcs:
        if b <= insn_rva < en:
            return b, en
    return None

# find __FUNCSIG__ strings
sigs = {}
for m in re.finditer(rb"[^\x00]{0,400}?__cdecl[^\x00]{0,600}", data):
    s = m.group().decode("latin1")
    rva = off2rva(m.start())
    if rva is not None:
        sigs.setdefault(s, rva)

# parse the objdump text for rip-relative comment refs
txt = open("/tmp/text.asm", encoding="latin1", errors="replace").read()
refre = re.compile(r"^\s*([0-9a-f]+):\s.*#\s*(0x[0-9a-f]+)\s*$", re.M)
# map target addr -> list of instruction addrs
tgt2insn = {}
for m in refre.finditer(txt):
    ia = IB + int(m.group(1), 16) if int(m.group(1), 16) < IB else int(m.group(1), 16)
    if ia < IB:
        ia += IB
    t = int(m.group(2), 16)
    tgt2insn.setdefault(t, []).append(ia)

out = []
A = out.append
named = {}
for s, rva in sorted(sigs.items(), key=lambda kv: kv[1]):
    addr = IB + rva
    insns = tgt2insn.get(addr, [])
    for ia in insns:
        o = owner(ia - IB)
        if o:
            named.setdefault(o[0], set()).add(s)
A("Function names recovered via embedded __FUNCSIG__ literals")
A("binary=%s  .pdata functions=%d  unique __FUNCSIG__=%d" % (PATH, len(funcs), len(sigs)))
A("")
A("%-14s %-14s %s" % ("start", "end", "recovered C++ name"))
for b, en, uw in funcs:
    if b in named:
        for s in sorted(named[b]):
            A("0x%08x  0x%08x  %s" % (IB + b, IB + en, s[:200]))
A("")
A("== .pdata function ranges with NO recovered name: %d ==" %
  (len(funcs) - len(named)))
json.dump({hex(IB + k): sorted(v) for k, v in named.items()},
          open("recon/functions.json", "w"), indent=1)
open("recon/functions.txt", "w").write("\n".join(out) + "\n")
print("funcs=%d named=%d sigs=%d" % (len(funcs), len(named), len(sigs)))