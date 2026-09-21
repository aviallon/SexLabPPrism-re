#!/usr/bin/env python3
"""Minimal PE32+ reader + MSVC RTTI recovery for SexLabPPrism.dll.

Read-only. Parses type descriptors (.?AV/.?AU), CompleteObjectLocators,
ClassHierarchyDescriptors, BaseClassArrays, BaseClassDescriptors and the
vtables that point at each COL. Emits recon/rtti.txt.

Every claim is traceable to a file offset / RVA printed in the output.
"""
import struct, sys, re, os, json

PATH = sys.argv[1] if len(sys.argv) > 1 else "artifacts/SexLabPPrism.dll"
data = open(PATH, "rb").read()

# ---- PE parse -------------------------------------------------------------
e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
assert data[e_lfanew:e_lfanew+4] == b"PE\0\0"
coff = e_lfanew + 4
num_sec, = struct.unpack_from("<H", data, coff + 2)
opt_size, = struct.unpack_from("<H", data, coff + 16)
opt = coff + 20
magic, = struct.unpack_from("<H", data, opt)
assert magic == 0x20b
image_base, = struct.unpack_from("<Q", data, opt + 24)
sec_align, = struct.unpack_from("<I", data, opt + 32)
file_align, = struct.unpack_from("<I", data, opt + 36)
num_dd, = struct.unpack_from("<I", data, opt + 108)
dd = opt + 112

sections = []
sectab = opt + opt_size
for i in range(num_sec):
    off = sectab + i * 40
    name = data[off:off+8].rstrip(b"\0").decode("latin1")
    vsize, vaddr, rsize, raw = struct.unpack_from("<IIII", data, off + 8)
    sections.append((name, vaddr, vsize, raw, rsize))

def rva_to_off(rva):
    for name, vaddr, vsize, raw, rsize in sections:
        if vaddr <= rva < vaddr + max(vsize, rsize):
            return raw + (rva - vaddr)
    return None

def off_to_rva(off):
    for name, vaddr, vsize, raw, rsize in sections:
        if raw <= off < raw + rsize:
            return vaddr + (off - raw)
    return None

def rd_rva(rva, n):
    o = rva_to_off(rva)
    if o is None or o + n > len(data):
        return None
    return data[o:o+n]

def u32(rva):
    b = rd_rva(rva, 4)
    return struct.unpack("<I", b)[0] if b else None

def u64(rva):
    b = rd_rva(rva, 8)
    return struct.unpack("<Q", b)[0] if b else None

def cstr_off(off, maxlen=512):
    end = data.find(b"\0", off, off + maxlen)
    if end < 0:
        return None
    return data[off:end].decode("latin1", "replace")

# ---- data directories ----------------------------------------------------
def dd_entry(i):
    return struct.unpack_from("<II", data, dd + i * 8)

exp_rva, exp_size = dd_entry(0)
imp_rva, imp_size = dd_entry(1)

# ---- 1. type descriptors -------------------------------------------------
# TypeDescriptor: { vfptr(8); spare(8); char name[]; }  -> name at TD_rva+16
tds = {}          # rva of TypeDescriptor -> name
name_to_rva = {}
pat = re.compile(rb"\.\?[AU][VU][A-Za-z0-9_@?$.\-]*@@")
for name, vaddr, vsize, raw, rsize in sections:
    if name not in (".rdata", ".data"):
        continue
    blob = data[raw:raw+rsize]
    for m in pat.finditer(blob):
        noff = raw + m.start()
        rva = off_to_rva(noff)
        s = cstr_off(noff)
        if not s or s != data[noff:noff+len(s)].decode("latin1", "replace"):
            continue
        s = s.rstrip("@") + "@@" if False else s
        td_rva = rva - 16
        # sanity: TD must live in a section, and vfptr is 0 for statically-built RTTI
        if rva_to_off(td_rva) is None:
            continue
        tds[td_rva] = s
        name_to_rva.setdefault(s, td_rva)

# ---- 2. CompleteObjectLocators ------------------------------------------
# x64: sig(4) offset(4) cdOffset(4) pTypeDescriptor(4 RVA) pClassDescriptor(4 RVA) pSelf(4 RVA)
cols = {}   # col_rva -> dict
for name, vaddr, vsize, raw, rsize in sections:
    if name != ".rdata":
        continue
    for off in range(raw, raw + rsize - 24, 4):
        sig, offset, cd = struct.unpack_from("<III", data, off)
        ptd, pcd, pself = struct.unpack_from("<III", data, off + 12)
        if sig not in (0, 1):
            continue
        loc_rva = off_to_rva(off)
        if loc_rva != pself:
            continue
        if ptd not in tds:
            continue
        if pcd == 0 or rva_to_off(pcd) is None:
            continue
        cols[loc_rva] = dict(sig=sig, offset=offset, cd=cd,
                             td=ptd, chd=pcd, self=pself)

# ---- 3. Class hierarchy --------------------------------------------------
chd_cache = {}
def parse_chd(rva):
    if rva in chd_cache:
        return chd_cache[rva]
    b = rd_rva(rva, 16)
    if not b:
        return None
    sig, attrs, nbase, bca = struct.unpack("<IIII", b)
    if sig != 0 or nbase > 64:
        return None
    out = dict(sig=sig, attrs=attrs, nbase=nbase, bca=bca, bases=[])
    if bca and rva_to_off(bca):
        for i in range(nbase):
            brva = u32(bca + i * 4)
            if brva is None:
                break
            bb = rd_rva(brva, 24)
            if not bb:
                break
            ptd, ncb, mdisp, pdisp, vdisp, battrs = struct.unpack("<IIiiii", bb) if False else (None,)*6
            ptd, ncb = struct.unpack_from("<II", bb, 0)
            mdisp, pdisp, vdisp = struct.unpack_from("<iii", bb, 8)
            battrs = struct.unpack_from("<I", bb, 20)[0]
            out["bases"].append(dict(bcd=brva, td=ptd, ncb=ncb,
                                     mdisp=mdisp, pdisp=pdisp, vdisp=vdisp,
                                     attrs=battrs,
                                     name=tds.get(ptd, "<unknown TD 0x%x>" % ptd)))
    chd_cache[rva] = out
    return out

# ---- 4. vtables pointing at COLs ----------------------------------------
vtables = {}
for name, vaddr, vsize, raw, rsize in sections:
    if name not in (".rdata", ".data"):
        continue
    for off in range(raw, raw + rsize - 8, 8):
        v = struct.unpack_from("<Q", data, off)[0]
        if v < image_base or v >= image_base + 0x1000000:
            continue
        rva = v - image_base
        if rva in cols:
            vtables.setdefault(rva, []).append(off_to_rva(off))

# ---- report --------------------------------------------------------------
out = []
A = out.append
A("MSVC RTTI recovery for %s" % PATH)
A("sha256(first32)=%s size=%d" % (__import__("hashlib").sha256(data).hexdigest()[:32], len(data)))
A("image_base=0x%x sections=%s" % (image_base, [(n, hex(v), hex(sz)) for n, v, sz, r, rs in sections]))
A("")
A("== COUNTS ==")
A("type_descriptors (unique RVA) : %d" % len(tds))
A("complete_object_locators     : %d" % len(cols))
A("unique class_hierarchy_descr : %d" % len({c['chd'] for c in cols.values()}))
A("vtables (sites pointing at COL): %d" % sum(len(v) for v in vtables.values()))
A("")

A("== TYPE DESCRIPTORS (name @ TD rva) ==")
for rva, s in sorted(tds.items(), key=lambda kv: kv[1].lower()):
    A("%-70s TD=0x%08x" % (s[:70], rva))

A("")
A("== CLASS HIERARCHIES (COL -> base chain, outermost-first) ==")
for col_rva, c in sorted(cols.items()):
    chd = parse_chd(c["chd"])
    tdname = tds.get(c["td"], "?")
    vts = vtables.get(col_rva, [])
    A("COL 0x%08x sig=%d offset=%d cd=%d  type=%s" % (col_rva, c["sig"], c["offset"], c["cd"], tdname))
    A("    vtable sites: %s" % (", ".join("0x%x" % v for v in vts) if vts else "NONE FOUND"))
    if chd:
        A("    CHD 0x%08x attrs=0x%x nbase=%d bca=0x%x" % (c["chd"], chd["attrs"], chd["nbase"], chd["bca"]))
        for b in chd["bases"]:
            A("      base: %-60s bcd=0x%08x ncb=%d mdisp=%d pdisp=%d vdisp=%d attrs=0x%x"
              % (b["name"][:60], b["bcd"], b["ncb"], b["mdisp"], b["pdisp"], b["vdisp"], b["attrs"]))
    else:
        A("    CHD unparsable at 0x%08x" % c["chd"])

A("")
A("== MULTIPLE INHERITANCE / REPEATED BASE NAMES ==")
from collections import Counter
bc = Counter()
for c in cols.values():
    chd = parse_chd(c["chd"])
    if chd:
        for b in chd["bases"]:
            bc[b["name"]] += 1
for nm, n in bc.most_common(40):
    A("%-70s x%d" % (nm[:70], n))

# machine-readable
j = dict(type_descriptors={hex(k): v for k, v in tds.items()},
         cols={hex(k): v for k, v in cols.items()},
         vtables={hex(k): v for k, v in vtables.items()},
         count_types=len(tds), count_cols=len(cols))
open("recon/rtti.json", "w").write(json.dumps(j, indent=1, sort_keys=True))

open("recon/rtti.txt", "w").write("\n".join(out) + "\n")
print("wrote recon/rtti.txt (%d lines); types=%d cols=%d vtables=%d"
      % (len(out), len(tds), len(cols), sum(len(v) for v in vtables.values())))