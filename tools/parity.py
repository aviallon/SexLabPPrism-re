#!/usr/bin/env python3
"""Per-function disassembly-parity harness for two Windows x64 PEs.

Compares an original closed-source DLL against a source-level rebuild:
  * PE metadata: sections, optional header, CORRECTLY parsed Rich header,
    .rsrc VERSIONINFO.
  * Function inventory from .pdata RUNTIME_FUNCTION entries PLUS a gap scan
    for leaf functions that carry no unwind info.  .text coverage is reported
    honestly (covered vs uncovered bytes).
  * ONE objdump run per binary over the whole .text, then split by range.
  * Normalised tokens (address-like immediates / RIP targets / branch targets
    canonicalised, trailing padding dropped) -> strict hash + raw form.
  * Three-pass pairing: exact hash, fuzzy structural, leftovers unmatched.
  * Per-function verdicts -> recon/parity/per-function.json
  * Human report -> recon/parity-baseline.md

Usage:
  python3 tools/parity.py [--orig A.dll] [--new B.dll] [--outdir recon/parity]
                          [--report recon/parity-baseline.md] [--self-test]

Dependencies: python3 + objdump (binutils).  Nothing else.
"""
from __future__ import annotations

import argparse
import bisect
import hashlib
import json
import os
import re
import struct
import subprocess
import sys
from collections import Counter, defaultdict, deque
from difflib import SequenceMatcher

# ---------------------------------------------------------------------------
# small helpers
# ---------------------------------------------------------------------------

def u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def u64(b, o):
    return struct.unpack_from("<Q", b, o)[0]


def sha1(text: str) -> str:
    return hashlib.sha1(text.encode("utf-8", "replace")).hexdigest()[:16]


# ---------------------------------------------------------------------------
# PE parsing
# ---------------------------------------------------------------------------

DATA_DIRS = ["export", "import", "resource", "exception", "security", "basereloc",
             "debug", "arch", "globalptr", "tls", "loadcfg", "boundimport", "iat",
             "delayimport", "clr", "reserved"]

# Rich header product -> friendly name (subset that matters for MSVC toolsets)
RICH_PRODUCTS = {
    0x0000: "Unknown", 0x0001: "Import0", 0x0002: "Linker510", 0x0003: "Cvtomf510",
    0x0004: "Linker600", 0x0005: "Cvtomf600", 0x0006: "Cvtres500",
    0x0007: "Utc11_Basic", 0x0008: "Utc11_C", 0x0009: "Utc12_Basic",
    0x000A: "Utc12_C", 0x000B: "Utc12_CPP", 0x000C: "AliasObj60",
    0x000D: "VisualBasic60", 0x000E: "Masm613", 0x000F: "Masm710",
    0x0010: "Linker511", 0x0011: "Cvtomf511", 0x0012: "Masm614",
    0x0013: "Linker512", 0x0014: "Cvtomf512", 0x0015: "Utc12_C_Std",
    0x0016: "Utc12_CPP_Std", 0x0017: "Utc12_C_Book", 0x0018: "Utc12_CPP_Book",
    0x0019: "Implib700", 0x001A: "Cvtomf700", 0x001B: "Utc13_Basic",
    0x001C: "Utc13_C", 0x001D: "Utc13_CPP", 0x001E: "Linker610",
    0x001F: "Cvtomf610", 0x0020: "Linker601", 0x0021: "Cvtomf601",
    0x0022: "Utc12_1_Basic", 0x0023: "Utc12_1_C", 0x0024: "Utc12_1_CPP",
    0x0025: "Linker620", 0x0026: "Cvtomf620", 0x0027: "AliasObj70",
    0x0028: "Linker621", 0x0029: "Cvtomf621", 0x002A: "Masm615",
    0x002B: "Utc13_LTCG_C", 0x002C: "Utc13_LTCG_CPP", 0x002D: "Masm620",
    0x002E: "ILAsm100", 0x002F: "Utc12_2_Basic", 0x0030: "Utc12_2_C",
    0x0031: "Utc12_2_CPP", 0x0032: "Utc12_2_C_Std", 0x0033: "Utc12_2_CPP_Std",
    0x0034: "Utc12_2_C_Book", 0x0035: "Utc12_2_CPP_Book", 0x0036: "Implib622",
    0x0037: "Cvtomf622", 0x0038: "Cvtres501", 0x0039: "Utc13_C_Std",
    0x003A: "Utc13_CPP_Std", 0x003B: "Cvtpgd1300", 0x003C: "Linker622",
    0x003D: "Linker700", 0x003E: "Export622", 0x003F: "Export700",
    0x0040: "Masm700", 0x0041: "Utc13_POGO_I_C", 0x0042: "Utc13_POGO_I_CPP",
    0x0043: "Utc13_POGO_O_C", 0x0044: "Utc13_POGO_O_CPP", 0x0045: "Cvtres700",
    0x0046: "Cvtres710p", 0x0047: "Linker710p", 0x0048: "Cvtomf710p",
    0x0049: "Export710p", 0x004A: "Implib710p", 0x004B: "Masm710p",
    0x004C: "Utc1310p_C", 0x004D: "Utc1310p_CPP", 0x004E: "Utc1310p_C_Std",
    0x004F: "Utc1310p_CPP_Std", 0x0050: "Utc1310p_LTCG_C",
    0x0051: "Utc1310p_LTCG_CPP", 0x0052: "Utc1310p_POGO_I_C",
    0x0053: "Utc1310p_POGO_I_CPP", 0x0054: "Utc1310p_POGO_O_C",
    0x0055: "Utc1310p_POGO_O_CPP", 0x0056: "Linker624", 0x0057: "Cvtomf624",
    0x0058: "Export624", 0x0059: "Implib624", 0x005A: "Linker710",
    0x005B: "Cvtomf710", 0x005C: "Export710", 0x005D: "Implib710",
    0x005E: "Cvtres710", 0x005F: "Utc1310_C", 0x0060: "Utc1310_CPP",
    0x0061: "Utc1310_C_Std", 0x0062: "Utc1310_CPP_Std", 0x0063: "Utc1310_LTCG_C",
    0x0064: "Utc1310_LTCG_CPP", 0x0065: "Utc1310_POGO_I_C",
    0x0066: "Utc1310_POGO_I_CPP", 0x0067: "Utc1310_POGO_O_C",
    0x0068: "Utc1310_POGO_O_CPP", 0x0069: "AliasObj710",
    0x006A: "AliasObj710p", 0x006B: "Cvtpgd1310", 0x006C: "Cvtpgd1310p",
    0x006D: "Utc1400_C", 0x006E: "Utc1400_CPP", 0x006F: "Utc1400_C_Std",
    0x0070: "Utc1400_CPP_Std", 0x0071: "Utc1400_LTCG_C",
    0x0072: "Utc1400_LTCG_CPP", 0x0073: "Utc1400_POGO_I_C",
    0x0074: "Utc1400_POGO_I_CPP", 0x0075: "Utc1400_POGO_O_C",
    0x0076: "Utc1400_POGO_O_CPP", 0x0077: "Cvtpgd1400", 0x0078: "Linker800",
    0x0079: "Cvtomf800", 0x007A: "Export800", 0x007B: "Implib800",
    0x007C: "Cvtres800", 0x007D: "Masm800", 0x007E: "AliasObj800",
    0x007F: "PhoenixPrerelease", 0x0080: "Utc1400_CVTCIL_C",
    0x0081: "Utc1400_CVTCIL_CPP", 0x0082: "Utc1400_LTCG_MSIL",
    0x0083: "Utc1500_C", 0x0084: "Utc1500_CPP", 0x0085: "Utc1500_C_Std",
    0x0086: "Utc1500_CPP_Std", 0x0087: "Utc1500_CVTCIL_C",
    0x0088: "Utc1500_CVTCIL_CPP", 0x0089: "Utc1500_LTCG_C",
    0x008A: "Utc1500_LTCG_CPP", 0x008B: "Utc1500_POGO_I_C",
    0x008C: "Utc1500_POGO_I_CPP", 0x008D: "Utc1500_POGO_O_C",
    0x008E: "Utc1500_POGO_O_CPP", 0x0090: "Cvtpgd1500", 0x0091: "Linker900",
    0x0092: "Export900", 0x0093: "Implib900", 0x0094: "Cvtres900",
    0x0095: "Masm900", 0x0096: "AliasObj900", 0x0097: "Resource900",
    0x0098: "AliasObj1000", 0x0099: "Cvtpgd1600", 0x009A: "Cvtres1000",
    0x009B: "Export1000", 0x009C: "Implib1000", 0x009D: "Linker1000",
    0x009E: "Masm1000", 0x009F: "Phx1600_C", 0x00A0: "Phx1600_CPP",
    0x00A1: "Phx1600_CVTCIL_C", 0x00A2: "Phx1600_CVTCIL_CPP",
    0x00A3: "Phx1600_LTCG_C", 0x00A4: "Phx1600_LTCG_CPP",
    0x00A5: "Phx1600_POGO_I_C", 0x00A6: "Phx1600_POGO_I_CPP",
    0x00A7: "Phx1600_POGO_O_C", 0x00A8: "Phx1600_POGO_O_CPP",
    0x00A9: "Utc1600_C", 0x00AA: "Utc1600_CPP", 0x00AB: "Utc1600_CVTCIL_C",
    0x00AC: "Utc1600_CVTCIL_CPP", 0x00AD: "Utc1600_LTCG_C",
    0x00AE: "Utc1600_LTCG_CPP", 0x00AF: "Utc1600_POGO_I_C",
    0x00B0: "Utc1600_POGO_I_CPP", 0x00B1: "Utc1600_POGO_O_C",
    0x00B2: "Utc1600_POGO_O_CPP", 0x00B3: "AliasObj1010",
    0x00B4: "Cvtpgd1700", 0x00B5: "Cvtres1010", 0x00B6: "Export1010",
    0x00B7: "Implib1010", 0x00B8: "Linker1010", 0x00B9: "Masm1010",
    0x00BA: "Utc1700_C", 0x00BB: "Utc1700_CPP", 0x00BC: "Utc1700_CVTCIL_C",
    0x00BD: "Utc1700_CVTCIL_CPP", 0x00BE: "Utc1700_LTCG_C",
    0x00BF: "Utc1700_LTCG_CPP", 0x00C0: "Utc1700_POGO_I_C",
    0x00C1: "Utc1700_POGO_I_CPP", 0x00C2: "Utc1700_POGO_O_C",
    0x00C3: "Utc1700_POGO_O_CPP", 0x00C4: "AliasObj1020",
    0x00C5: "Cvtpgd1800", 0x00C6: "Cvtres1020", 0x00C7: "Export1020",
    0x00C8: "Implib1020", 0x00C9: "Linker1020", 0x00CA: "Masm1020",
    0x00CB: "Utc1800_C", 0x00CC: "Utc1800_CPP", 0x00CD: "Utc1800_CVTCIL_C",
    0x00CE: "Utc1800_CVTCIL_CPP", 0x00CF: "Utc1800_LTCG_C",
    0x00D0: "Utc1800_LTCG_CPP", 0x00D1: "Utc1800_POGO_I_C",
    0x00D2: "Utc1800_POGO_I_CPP", 0x00D3: "Utc1800_POGO_O_C",
    0x00D4: "Utc1800_POGO_O_CPP", 0x00D5: "AliasObj1030",
    0x00D6: "Cvtpgd1900", 0x00D7: "Cvtres1030", 0x00D8: "Export1030",
    0x00D9: "Implib1030", 0x00DA: "Linker1030", 0x00DB: "Masm1030",
    0x00DC: "Utc1900_C", 0x00DD: "Utc1900_CPP", 0x00DE: "Utc1900_CVTCIL_C",
    0x00DF: "Utc1900_CVTCIL_CPP", 0x00E0: "Utc1900_LTCG_C",
    0x00E1: "Utc1900_LTCG_CPP", 0x00E2: "Utc1900_POGO_I_C",
    0x00E3: "Utc1900_POGO_I_CPP", 0x00E4: "Utc1900_POGO_O_C",
    0x00E5: "Utc1900_POGO_O_CPP", 0x00E6: "AliasObj1040",
    0x00E7: "Cvtpgd1910", 0x00E8: "Cvtres1040", 0x00E9: "Export1040",
    0x00EA: "Implib1040", 0x00EB: "Linker1040", 0x00EC: "Masm1040",
    0x00ED: "Utc1910_C", 0x00EE: "Utc1910_CPP", 0x00EF: "Utc1910_CVTCIL_C",
    0x00F0: "Utc1910_CVTCIL_CPP", 0x00F1: "Utc1910_LTCG_C",
    0x00F2: "Utc1910_LTCG_CPP", 0x00F3: "Utc1910_POGO_I_C",
    0x00F4: "Utc1910_POGO_I_CPP", 0x00F5: "Utc1910_POGO_O_C",
    0x00F6: "Utc1910_POGO_O_CPP", 0x00F7: "AliasObj1050",
    0x00F8: "Cvtpgd1920", 0x00F9: "Cvtres1050", 0x00FA: "Export1050",
    0x00FB: "Implib1050", 0x00FC: "Linker1050", 0x00FD: "Masm1050",
    0x00FE: "Utc1920_C", 0x00FF: "Utc1920_CPP", 0x0100: "Utc1920_CVTCIL_C",
    0x0101: "Utc1920_CVTCIL_CPP", 0x0102: "Utc1920_LTCG_C",
    0x0103: "Utc1920_LTCG_CPP", 0x0104: "Utc1920_POGO_I_C",
    0x0105: "Utc1920_POGO_I_CPP", 0x0106: "Utc1920_POGO_O_C",
    0x0107: "Utc1920_POGO_O_CPP",
}


class PE:
    def __init__(self, path):
        self.path = path
        self.data = open(path, "rb").read()
        d = self.data
        e = u32(d, 0x3C)
        assert d[e:e + 4] == b"PE\0\0", "not a PE"
        self.e_lfanew = e
        coff = e + 4
        self.machine = u16(d, coff)
        nsec = u16(d, coff + 2)
        optsz = u16(d, coff + 16)
        self.nsec = nsec
        opt = coff + 20
        self.opt = opt
        self.magic = u16(d, opt)
        assert self.magic == 0x20B, "not PE32+"
        self.linker = (d[opt + 2], d[opt + 3])
        self.entry = u32(d, opt + 16)
        self.image_base = u64(d, opt + 24)
        self.section_align = u32(d, opt + 32)
        self.file_align = u32(d, opt + 36)
        self.os_ver = (u16(d, opt + 40), u16(d, opt + 42))
        self.image_ver = (u16(d, opt + 44), u16(d, opt + 46))
        self.subsystem_ver = (u16(d, opt + 48), u16(d, opt + 50))
        self.size_of_image = u32(d, opt + 56)
        self.headers_size = u32(d, opt + 60)
        self.checksum = u32(d, opt + 64)
        self.subsystem = u16(d, opt + 68)
        self.dll_chars = u16(d, opt + 70)
        self.stack_reserve = u64(d, opt + 72)
        self.stack_commit = u64(d, opt + 80)
        self.heap_reserve = u64(d, opt + 88)
        self.heap_commit = u64(d, opt + 96)
        self.num_dd = u32(d, opt + 108)
        dd = opt + 112
        self.dirs = {}
        for i in range(min(self.num_dd, 16)):
            rva, sz = u32(d, dd + i * 8), u32(d, dd + i * 8 + 4)
            self.dirs[DATA_DIRS[i]] = (rva, sz)
        # sections
        secoff = opt + optsz
        self.sections = []
        for i in range(nsec):
            s = secoff + i * 40
            name = d[s:s + 8].rstrip(b"\0").decode("latin1")
            vsz, va, rsz, ra = (u32(d, s + 8), u32(d, s + 12),
                                u32(d, s + 16), u32(d, s + 20))
            chars = u32(d, s + 36)
            self.sections.append({"name": name, "va": va, "vsize": vsz,
                                  "raw": ra, "rsize": rsz, "chars": chars})
        self.rich = self._parse_rich()
        self.version = self._parse_version()

    # ---- RVA mapping -----------------------------------------------------
    def rva2off(self, rva):
        for s in self.sections:
            if s["va"] <= rva < s["va"] + max(s["vsize"], s["rsize"]):
                if s["raw"] == 0:
                    return None
                off = s["raw"] + (rva - s["va"])
                if off < len(self.data):
                    return off
        return None

    def off2rva(self, off):
        for s in self.sections:
            if s["raw"] and s["raw"] <= off < s["raw"] + s["rsize"]:
                return s["va"] + (off - s["raw"])
        return None

    def read_at_rva(self, rva, n):
        off = self.rva2off(rva)
        if off is None:
            return b""
        return self.data[off:off + n]

    def section(self, name):
        for s in self.sections:
            if s["name"] == name:
                return s
        return None

    # ---- Rich header (CORRECT) ------------------------------------------
    def _parse_rich(self):
        d = self.data
        rich = d.find(b"Rich", 0x40, 0x1000)
        if rich < 0:
            return None
        key = u32(d, rich + 4)
        # scan backwards for decoded 'DanS' (0x536e6144), stepping (comp,count)
        dans = None
        p = rich - 8
        while p > 0x3F:
            comp = u32(d, p) ^ key
            if comp == 0x536E6144:
                dans = p
                break
            p -= 8
        entries = []
        if dans is not None:
            q = dans + 4  # 3 pad dwords follow
            q += 12
            while q + 7 < rich:
                comp = u32(d, q) ^ key
                count = u32(d, q + 4) ^ key
                entries.append((comp, count))
                q += 8
        return {"offset": rich, "key": key, "dans_offset": dans,
                "entries": entries,
                "comp_ids": [e[0] for e in entries]}

    # ---- .rsrc VERSIONINFO ----------------------------------------------
    def _parse_version(self):
        if "resource" not in self.dirs:
            return None
        root_rva, _ = self.dirs["resource"]
        if not root_rva:
            return None
        off = self.rva2off(root_rva)
        if off is None:
            return None
        d = self.data

        def read_dir(o):
            if o < 0 or o + 16 > len(d):
                return []
            n_named = u16(d, o + 12)
            n_id = u16(d, o + 14)
            ents = []
            for i in range(n_named + n_id):
                e = o + 16 + i * 8
                if e + 8 > len(d):
                    break
                name = u32(d, e)
                sub = u32(d, e + 4)
                ents.append((name, sub))
            return ents

        mask = 0x7FFFFFFF
        result = {"types": []}
        for name, sub in read_dir(off):
            if name != 16:  # RT_VERSION
                continue
            for nm2, sub2 in read_dir(off + (sub & mask)):
                # level 3 entries point straight at IMAGE_RESOURCE_DATA_ENTRY
                for nm3, sub3 in read_dir(off + (sub2 & mask)):
                    data_ent = off + (sub3 & mask)
                    data_rva = u32(d, data_ent)
                    size = u32(d, data_ent + 4)
                    vbase = self.rva2off(data_rva)
                    if vbase is None:
                        continue
                    result["types"].append({
                        "name_id": nm2,
                        "lang": nm3,
                        "blocks": self._parse_vsversion(vbase, size)})
        return result

    def _parse_vsversion(self, base, total):
        d = self.data
        strings = {}
        fixed = {}

        def align4(x):
            return (x + 3) & ~3

        def walk(off):
            if off + 6 > len(d):
                return off
            wLength = u16(d, off)
            wValueLength = u16(d, off + 2)
            wType = u16(d, off + 4)
            key, after = self._read_wstr(off + 6)
            p = align4(after)
            end = off + wLength if wLength else len(d)
            if key == "VS_VERSION_INFO":
                if wValueLength >= 52:
                    ffi = d[p:p + 52]
                    (sig, sv, fvMS, fvLS, pvMS, pvLS) = struct.unpack_from("<IIIIII", ffi, 0)
                    fixed.update({
                        "signature": hex(sig), "struc_version": f"{sv>>16}.{sv&0xffff}",
                        "file_version": f"{fvMS>>16}.{fvMS&0xffff}.{fvLS>>16}.{fvLS&0xffff}",
                        "product_version": f"{pvMS>>16}.{pvMS&0xffff}.{pvLS>>16}.{pvLS&0xffff}",
                        "file_flags": hex(u32(ffi, 28)),
                        "file_os": hex(u32(ffi, 32)),
                        "file_type": hex(u32(ffi, 36)),
                    })
                p = align4(p + wValueLength)
            elif wValueLength and wType == 1:
                s, after2 = self._read_wstr(p)
                strings[key] = s
                p = align4(after2)
            elif wValueLength:
                p = align4(p + wValueLength)
            while p + 2 <= end and p < len(d):
                clen = u16(d, p)
                if clen == 0:
                    break
                walk(p)
                p = align4(p + clen)
            return end

        walk(base)
        return {"fixed": fixed, "strings": strings}

    def _read_wstr(self, off):
        d = self.data
        end = off
        while end + 1 < len(d) and d[end:end + 2] != b"\0\0":
            end += 2
        return d[off:end].decode("utf-16le", "replace"), end + 2

    # ---- pdata -----------------------------------------------------------
    def parse_pdata(self):
        rva, sz = self.dirs.get("exception", (0, 0))
        if not rva or sz < 12:
            return []
        out = []
        for i in range(sz // 12):
            o = self.rva2off(rva + i * 12)
            if o is None:
                break
            b, e, uw = struct.unpack_from("<III", self.data, o)
            if b == 0 and e == 0:
                continue
            out.append((self.image_base + b, self.image_base + e, uw))
        out.sort()
        return out

    def exports(self):
        rva, sz = self.dirs.get("export", (0, 0))
        if not rva:
            return []
        d = self.data
        o = self.rva2off(rva)
        if o is None:
            return []
        base_ord = u32(d, o + 16)
        nfunc = u32(d, o + 20)
        nname = u32(d, o + 24)
        af = u32(d, o + 28)
        an = u32(d, o + 32)
        ao = u32(d, o + 36)
        res = []
        for i in range(nname):
            no = self.rva2off(an + i * 4)
            if no is None:
                continue
            name_rva = u32(d, no)
            so = self.rva2off(name_rva)
            if so is None:
                continue
            name = d[so:so + 256].split(b"\0")[0].decode("latin1")
            fo = self.rva2off(ao + i * 2)
            ordi = u16(d, fo) if fo is not None else 0
            fo2 = self.rva2off(af + ordi * 4)
            frva = u32(d, fo2) if fo2 is not None else 0
            res.append({"name": name, "ordinal": base_ord + ordi, "rva": frva})
        return res


# ---------------------------------------------------------------------------
# objdump disassembly
# ---------------------------------------------------------------------------

INS_RE = re.compile(r"^\s*([0-9a-fA-F]+):\s*((?:[0-9a-fA-F]{2}\s)+)(.*)$")
COMMENT_RE = re.compile(r"#\s*(0x[0-9a-fA-F]+)")
HEX_RE = re.compile(r"0x[0-9a-fA-F]+")

BRANCH_MNEMS = {
    "call", "jmp", "ja", "jae", "jb", "jbe", "jc", "je", "jg", "jge", "jl",
    "jle", "jne", "jno", "jnp", "jns", "jo", "jp", "js", "jz", "jnz",
    "loop", "loope", "loopne", "loopnz", "loopz",
}


def run_objdump(path, text_va=None, text_vsize=None, keep=None):
    cmd = ["objdump", "-d", "-Mintel", "--insn-width=16", "-j", ".text", path]
    env = dict(os.environ)
    env["LC_ALL"] = "C"
    proc = subprocess.run(cmd, capture_output=True, env=env)
    out = proc.stdout.decode("latin1", "replace")
    if keep:
        try:
            open(keep, "w").write(out)
        except OSError:
            pass
    return out


def parse_objdump(text, image_base):
    insns = []
    for line in text.splitlines():
        m = INS_RE.match(line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        bhex = "".join(m.group(2).split())
        nb = len(bhex) // 2
        rest = m.group(3).strip()
        if not rest:
            if insns and addr == insns[-1]["addr"] + insns[-1]["size"]:
                insns[-1]["size"] += nb
                insns[-1]["bytes"] += bhex
                continue
            # lone bytes line (should not happen in a synchronised run)
            continue
        cm = COMMENT_RE.search(rest)
        comment = int(cm.group(1), 16) if cm else None
        code = rest.split("#")[0].strip()
        parts = code.split(None, 1)
        mn = parts[0].lower()
        ops = parts[1].strip() if len(parts) > 1 else ""
        # branch target if the operand is a bare address
        branch = None
        if mn in BRANCH_MNEMS:
            pm = re.match(r"^(0x[0-9a-fA-F]+)$", ops)
            if pm:
                branch = int(pm.group(1), 16)
        refs = set()
        if comment is not None:
            refs.add(comment)
        insns.append({"addr": addr, "size": nb, "mn": mn, "ops": ops,
                      "text": f"{mn} {ops}".strip(), "bytes": bhex,
                      "comment": comment, "branch": branch, "refs": refs})
    insns.sort(key=lambda i: i["addr"])
    return insns


# ---------------------------------------------------------------------------
# normalisation
# ---------------------------------------------------------------------------

def _split_ops(s):
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch == "[":
            depth += 1
        elif ch == "]":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur)
    return [x.strip() for x in out]


def _norm_operand(p, ins, fstart, fend, image_base, image_end, collect_refs):
    # RIP-relative memory operand
    m = re.search(r"\[rip([+-]0x[0-9a-fA-F]+)?\]", p)
    if m:
        disp = 0
        if m.group(1):
            disp = int(m.group(1), 16)
        tgt = ins["addr"] + ins["size"] + disp
        if collect_refs is not None:
            collect_refs.add(tgt)
        loc = f"L+0x{tgt - fstart:x}" if fstart <= tgt < fend else "EXT"
        return p[:m.start()] + f"[rip+{loc}]" + p[m.end():]
    # SIB / base+index memory operand carrying a displacement, e.g.
    # [rcx+rdx*4+0x5bab0]. A displacement that lands inside the image's RVA
    # range is a LINK-TIME SYMBOL DIFFERENCE - the original and our rebuild
    # place the same data at different RVAs, so the constant differs while the
    # code is identical (measured: 0x18000d194 vs our analogue differ only in
    # such displacements, ratio 0.941 -> 1.0 once canonicalised). Small
    # displacements stay literal: a struct field offset is part of the code's
    # meaning and must not be normalised away.
    m = re.search(r"\[[^\]]*[+-](0x[0-9a-fA-F]+)\]", p)
    if m:
        v = int(m.group(1), 16)
        if 0 < v < (image_end - image_base):
            if collect_refs is not None:
                collect_refs.add(image_base + v)
            return p[:m.start(1)] + "DISP_RVA" + p[m.end(1):]
    # absolute memory operand [0x...]
    m = re.search(r"\[(0x[0-9a-fA-F]+)\]", p)
    if m:
        if collect_refs is not None:
            v = int(m.group(1), 16)
            if image_base <= v < image_end:
                collect_refs.add(v)
        return p[:m.start()] + "[MEM]" + p[m.end():]
    # bare branch target
    m = re.match(r"^(0x[0-9a-fA-F]+)$", p)
    if m and ins["mn"] in BRANCH_MNEMS:
        tgt = int(m.group(1), 16)
        if collect_refs is not None:
            collect_refs.add(tgt)
        if fstart <= tgt < fend:
            return f"L+0x{tgt - fstart:x}"
        return "EXT"
    # address-like immediate
    if m:
        v = int(m.group(1), 16)
        if collect_refs is not None and image_base <= v < image_end:
            collect_refs.add(v)
        if image_base <= v < image_end:
            return "IMM_ADDR"
        return p
    # numeric immediates inside larger operands (e.g. "QWORD PTR [rsp+0x8],0x180...")
    def repl(mm):
        v = int(mm.group(0), 16)
        if collect_refs is not None and image_base <= v < image_end:
            collect_refs.add(v)
        return "IMM_ADDR" if image_base <= v < image_end else mm.group(0)
    return HEX_RE.sub(repl, p)


def normalise_insn(ins, fstart, fend, image_base, image_end, collect_refs=None):
    ops = ins["ops"]
    parts = _split_ops(ops)
    nops = [_norm_operand(p, ins, fstart, fend, image_base, image_end, collect_refs)
            for p in parts]
    return ins["mn"] + (" " + ",".join(nops) if nops else "")


PAD_MNEMS = ("nop", "int3", "int")


def normalise_function(func, image_base, image_end):
    fstart, fend = func["addr"], func["end"]
    tokens = []
    raw = []
    refs = set()
    for ins in func["insns"]:
        tok = normalise_insn(ins, fstart, fend, image_base, image_end, refs)
        tokens.append(tok)
        raw.append(ins["text"])
    # strip trailing padding
    while tokens and (tokens[-1].split(" ", 1)[0] in PAD_MNEMS):
        tokens.pop()
        raw.pop()
    return tokens, raw, refs


# ---------------------------------------------------------------------------
# function inventory
# ---------------------------------------------------------------------------

def build_functions(pe, insns):
    pdata = pe.parse_pdata()
    # MSVC C++ exception handling splits ONE source function into SEVERAL
    # RUNTIME_FUNCTION entries in .pdata (a prologue fragment, the body, and
    # cleanup fragments). Taking the first entry as the function extent made
    # multi-fragment functions read as a 7-8 instruction prologue and score
    # MISSING no matter how well the body matched - precisely the measurement
    # artefact found on CatalogFinish/CatalogPublish (0x18002a270 + 0x43 ==
    # 0x18002a29b, the next fragment's start). Contiguous fragments are one
    # function: merge them before anything is compared.
    merged = []
    for b, e, uw in sorted(pdata, key=lambda t: t[0]):
        if e <= b:
            continue
        # MSVC splits one function into several RUNTIME_FUNCTION entries; only a
        # CONTINUATION record carries UNW_FLAG_CHAININFO (0x4) in its UNWIND_INFO.
        # Merging on mere ADJACENCY chains different functions that happen to sit
        # next to each other with no padding, which inflates their measured size
        # (found on real cases: 0x180047140 read as 207 instructions instead of
        # 12). UNWIND_INFO byte 0 is Version:3 | Flags:5.
        chain = False
        o = pe.rva2off(uw) if isinstance(uw, int) else None
        if o is not None:
            flags = (pe.data[o] >> 3) & 0x1F
            chain = bool(flags & 0x4)  # UNW_FLAG_CHAININFO
        if merged and chain and merged[-1][1] == b:
            merged[-1] = (merged[-1][0], e, merged[-1][2])
        else:
            merged.append((b, e, uw))
    pdata = merged
    ranges = [(b, e) for b, e, _ in pdata if e > b]
    starts = [b for b, _ in ranges]
    tsec = pe.section(".text")
    text_lo = pe.image_base + tsec["va"]
    text_hi = text_lo + tsec["vsize"]

    funcs = []
    for b, e, uw in pdata:
        if e <= b:
            continue
        members = [i for i in insns if b <= i["addr"] < e]
        funcs.append({"addr": b, "end": e, "size": e - b, "source": "pdata",
                      "insns": members})
    # gap scan: instructions not inside any pdata range
    covered = [False] * len(insns)
    for idx, ins in enumerate(insns):
        for b, e in ranges:
            if b <= ins["addr"] < e:
                covered[idx] = True
                break
    uncovered = [ins for i, ins in enumerate(insns) if not covered[i]]
    # group runs and split at padding / aligned post-terminal boundaries
    runs = []
    cur = []
    for ins in uncovered:
        if cur and ins["addr"] != cur[-1]["addr"] + cur[-1]["size"]:
            runs.append(cur)
            cur = []
        if cur and cur[-1]["mn"] in PAD_MNEMS:
            runs.append(cur[:-1])
            cur = []
        if cur:
            prev = cur[-1]
            terminal = prev["mn"] in ("ret", "retn") or (
                prev["mn"] == "jmp" and prev.get("branch") is not None)
            if terminal and ins["addr"] % 16 == 0:
                runs.append(cur)
                cur = []
        cur.append(ins)
    if cur:
        runs.append(cur)
    for run in runs:
        run = [i for i in run if i["mn"] not in PAD_MNEMS]
        if not run:
            continue
        b = run[0]["addr"]
        e = run[-1]["addr"] + run[-1]["size"]
        funcs.append({"addr": b, "end": e, "size": e - b, "source": "gap",
                      "insns": run})
    funcs.sort(key=lambda f: f["addr"])

    # coverage
    td = pe.data[pe.rva2off(tsec["va"]):pe.rva2off(tsec["va"]) + tsec["vsize"]] \
        if pe.rva2off(tsec["va"]) is not None else b""
    cov_bytes = 0
    for b, e in ranges:
        lo = max(b, text_lo)
        hi = min(e, text_hi)
        if hi > lo:
            cov_bytes += hi - lo
    code_bytes = sum(i["size"] for i in uncovered)
    pad_bytes = len(td) - code_bytes - sum(
        min(e, text_hi) - max(b, text_lo) for b, e in ranges
        if min(e, text_hi) > max(b, text_lo))
    return funcs, {
        "text_lo": text_lo, "text_hi": text_hi, "text_vsize": tsec["vsize"],
        "pdata_entries": len(pdata), "pdata_covered_bytes": cov_bytes,
        "pdata_coverage_frac": cov_bytes / tsec["vsize"] if tsec["vsize"] else 0.0,
        "uncovered_bytes": tsec["vsize"] - cov_bytes,
        "uncovered_code_bytes": code_bytes,
        "uncovered_padding_bytes": max(0, tsec["vsize"] - cov_bytes - code_bytes),
        "gap_functions": sum(1 for f in funcs if f["source"] == "gap"),
    }


# ---------------------------------------------------------------------------
# names
# ---------------------------------------------------------------------------

SIG_RE = re.compile(rb"[\x20-\x7e]{0,320}__cdecl[\x20-\x7e]{0,700}")


def recover_sig_names(pe, funcs):
    d = pe.data
    sig_at = {}
    for m in SIG_RE.finditer(d):
        rva = pe.off2rva(m.start())
        if rva is None:
            continue
        s = m.group().decode("latin1", "replace")
        # trim leading garbage before a plausible name
        s = re.sub(r"^[^A-Za-z_]*", "", s)
        sig_at[pe.image_base + rva] = s
    named = 0
    for f in funcs:
        for ins in f["insns"]:
            for t in ins.get("refs", ()):  # type: ignore
                if t in sig_at:
                    f.setdefault("names", []).append(sig_at[t])
        if f.get("names"):
            named += 1
    return len(sig_at), named


def load_manifest(path):
    """recon/decompiled/MANIFEST.txt: '0xADDR\tlabel\t...' -> set of addresses."""
    addrs = set()
    if not os.path.exists(path):
        return addrs
    for line in open(path, encoding="utf-8", errors="replace"):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split("\t")
        try:
            addrs.add(int(parts[0], 16))
        except (ValueError, IndexError):
            continue
    return addrs


def load_name_json(path, pe):
    if not os.path.exists(path):
        return {}, 0
    try:
        raw = json.load(open(path))
    except Exception:
        return {}, 0
    out = {}
    for k, v in raw.items():
        try:
            addr = int(k, 16)
        except ValueError:
            continue
        out[addr] = v if isinstance(v, list) else [v]
    return out, len(out)


# ---------------------------------------------------------------------------
# classification
# ---------------------------------------------------------------------------

PLUGIN_NAME_MARKERS = [
    "Papyrus_", "InputSink", "MenuVisibilitySink", "FocusRecovery", "CreateViews",
    "OnMessage", "ApplyConsoleVisibility", "ApplyPresentation",
    "ApplyVanillaHUDVisibility", "InvokeOn", "OnConsoleMessage", "RequestSearchInput",
    "SendAction", "DispatchAction", "PublishCatalogToUi", "SetCollapsed",
    "HandleFocusHotkey", "BeginSceneSession", "PublishSceneState", "PublishCompatible",
    "CatalogBegin", "CatalogAppend", "CatalogPackage", "CatalogFinish",
    "IsCatalogReady", "GetCatalogCount", "CatalogPublish", "SetSearchQuery",
    "IsFreeCameraActive", "Register_Impl", "SKSEPlugin_Load", "SKSEPlugin_Query",
    "Papyrus::Natives", "NotImplemented", "CreateViews", "SetCollapsed",
]
LIBRARY_NAME_MARKERS = [
    "REL::", "SKSE::", "RE::", "std::", "spdlog", "fmt::", "@std@@", "@spdlog@@",
    "operator new", "operator delete", "_STL", "commonlib", "CommonLib",
    "vcruntime", "memcpy", "memmove", "memset", "malloc", "free", "exception@",
    "type_info", "basic_string", "vector@", "unordered", "shared_ptr",
]

PLUGIN_STRING_MARKERS = [
    b"SexLabPPrism", b"SexLabPrism", b"SLP_Prism", b"controller-0.6.1",
    b"InputSink", b"MenuVisibilitySink", b"FocusRecovery", b"versionlib-",
    b"failed to open address library", b"Unsupported address library format",
    b"SexLabP+", b"SexLab P+", b"PrismUI", b"prism",
]


def plugin_ref_addresses(pe):
    d = pe.data
    addrs = set()
    for marker in PLUGIN_STRING_MARKERS:
        start = 0
        while True:
            i = d.find(marker, start)
            if i < 0:
                break
            rva = pe.off2rva(i)
            if rva is not None:
                addrs.add(pe.image_base + rva)
                if i >= 16:
                    addrs.add(pe.image_base + rva - 16)  # RTTI type descriptor base
            start = i + 1
    return addrs


def classify(pe, funcs, name_map, exports, plugin_refs, extra_addrs=()):
    by_addr = {f["addr"]: f for f in funcs}
    for f in funcs:
        names = list(name_map.get(f["addr"], []))
        for n in f.get("names", []):
            if n not in names:
                names.append(n)
        f["names"] = names
        f["name"] = names[0] if names else None
        f["refs"] = set()
    # gather refs (already collected partly in normalise; recompute cheaply)
    for f in funcs:
        refs = set()
        for ins in f["insns"]:
            refs |= ins.get("refs", set())
            if ins.get("branch") and pe.image_base <= ins["branch"] < pe.image_base + pe.size_of_image:
                refs.add(ins["branch"])
            for mm in HEX_RE.findall(ins["ops"]):
                v = int(mm, 16)
                if pe.image_base <= v < pe.image_base + pe.size_of_image:
                    refs.add(v)
        f["refs"] = refs

    def matches(name, markers):
        return name is not None and any(m in name for m in markers)

    def library_named(name):
        # test only the function's own qualified name, not return or parameter types
        if not name:
            return False
        m = re.search(r"__cdecl\s+(.+)", name)
        scope = (m.group(1) if m else name).split("(", 1)[0]
        return any(lm in scope for lm in LIBRARY_NAME_MARKERS)

    export_addrs = {pe.image_base + e["rva"] for e in exports}

    # string address of "SexLabPrismNative" (registration function refs it)
    reg_strings = set()
    for marker in (b"SexLabPrismNative", b"SexLabPrismNative"):
        i = pe.data.find(marker)
        while i >= 0:
            rva = pe.off2rva(i)
            if rva is not None:
                reg_strings.add(pe.image_base + rva)
            i = pe.data.find(marker, i + 1)

    extra = set(extra_addrs)
    initial = set()
    for f in funcs:
        if f["addr"] in export_addrs or f["addr"] in extra:
            initial.add(f["addr"])
        if matches(f["name"], PLUGIN_NAME_MARKERS):
            initial.add(f["addr"])
        if f["refs"] & plugin_refs or f["refs"] & reg_strings:
            initial.add(f["addr"])

    # BFS over direct calls, not descending into library-named functions
    reached = set(initial)
    q = deque(initial)
    while q:
        a = q.popleft()
        f = by_addr.get(a)
        if f is None:
            continue
        for ins in f["insns"]:
            if ins["mn"] == "call" and ins.get("branch") in by_addr:
                t = ins["branch"]
                if t in reached:
                    continue
                tf = by_addr[t]
                if library_named(tf["name"]):
                    continue
                reached.add(t)
                q.append(t)

    for f in funcs:
        if library_named(f["name"]):
            f["tier"], f["basis"] = "library", "name:library"
        elif f["addr"] in export_addrs:
            f["tier"], f["basis"] = "plugin", "export"
        elif matches(f["name"], PLUGIN_NAME_MARKERS):
            f["tier"], f["basis"] = "plugin", "name:plugin"
        elif f["refs"] & plugin_refs:
            f["tier"], f["basis"] = "plugin", "ref:plugin-string"
        elif f["refs"] & reg_strings:
            f["tier"], f["basis"] = "plugin", "ref:registration-table"
        elif f["addr"] in extra:
            f["tier"], f["basis"] = "plugin", "known-native-address"
        elif f["addr"] in reached:
            f["tier"], f["basis"] = "plugin", "reachable"
        else:
            f["tier"], f["basis"] = "library", "unreached"
    return funcs


# ---------------------------------------------------------------------------
# comparison
# ---------------------------------------------------------------------------

EXACT, CLOSE, SIMILAR, DIFFERENT = "EXACT", "CLOSE", "SIMILAR", "DIFFERENT"
MISSING_NEW, MISSING_ORIG = "MISSING-NEW", "MISSING-ORIG"

FUZZY_MIN = 0.40      # below this we do not force a pair
SIMILAR_MIN = 0.60
CLOSE_MIN = 0.90


def verdict_for(ratio, exact=False):
    if exact:
        return EXACT
    if ratio >= CLOSE_MIN:
        return CLOSE
    if ratio >= SIMILAR_MIN:
        return SIMILAR
    return DIFFERENT


def prepare(funcs, image_base, image_size):
    image_end = image_base + image_size
    for f in funcs:
        tokens, raw, refs = normalise_function(f, image_base, image_end)
        f["tokens"] = tokens
        f["raw_tokens"] = raw
        f["icount"] = len(tokens)
        f["mnem"] = Counter(t.split(" ", 1)[0] for t in tokens)
        f["strict_hash"] = sha1("\n".join(tokens))
        f["raw_hash"] = sha1("\n".join(raw))
        # full byte string incl. trailing padding (for raw PE inspection)
        f["byte_str"] = "".join(i.get("bytes", "") for i in f["insns"])
        # bytes of the instructions that actually survive normalisation, i.e.
        # the same instruction window the token stream is built from.  This is
        # the correct basis for a byte-level equality claim: normalisation
        # deliberately drops trailing nop/int3 padding.
        f["code_bytes"] = "".join(i.get("bytes", "")
                                   for i in f["insns"][:len(tokens)])
        f["pad_insns"] = len(f["insns"]) - len(tokens)
        f["refs"] |= refs
    return funcs


def pair_functions(A, B):
    """Returns (pairs, unmatchedA, unmatchedB, stats)."""
    pairs = []
    ma = {}
    mb = {}
    for i, f in enumerate(A):
        ma.setdefault(f["strict_hash"], []).append(i)
    for j, f in enumerate(B):
        mb.setdefault(f["strict_hash"], []).append(j)
    usedA, usedB = set(), set()
    exact_raw = 0
    for h, ii in ma.items():
        jj = mb.get(h)
        if not jj:
            continue
        for i, j in zip(sorted(ii), sorted(jj)):
            usedA.add(i)
            usedB.add(j)
            pairs.append({"i": i, "j": j, "ratio": 1.0, "verdict": EXACT,
                          "pass": "exact", "raw_equal": A[i]["raw_hash"] == B[j]["raw_hash"]})
            if A[i]["raw_hash"] == B[j]["raw_hash"]:
                exact_raw += 1
    remA = [i for i in range(len(A)) if i not in usedA]
    remB = [j for j in range(len(B)) if j not in usedB]

    candidates = []
    for i in remA:
        ai = A[i]
        best = None
        for j in remB:
            bj = B[j]
            na, nb = ai["icount"], bj["icount"]
            if abs(na - nb) > max(5, 0.35 * max(na, nb, 1)):
                continue
            inter = sum((ai["mnem"] & bj["mnem"]).values())
            tot = max(ai["mnem"].total(), bj["mnem"].total(), 1)
            sim = inter / tot
            if sim < 0.45:
                continue
            r = SequenceMatcher(None, ai["tokens"], bj["tokens"], autojunk=False).ratio()
            if r < FUZZY_MIN:
                continue
            # reject low-confidence pairs that do not also satisfy a tight signature
            if r < SIMILAR_MIN:
                if abs(na - nb) > 2 or sim < 0.70:
                    continue
            if best is None or r > best[0]:
                best = (r, j)
        if best is not None:
            candidates.append((best[0], i, best[1], "fuzzy"))
    candidates.sort(key=lambda x: -x[0])
    for r, i, j, p in candidates:
        if i in usedA or j in usedB:
            continue
        usedA.add(i)
        usedB.add(j)
        pairs.append({"i": i, "j": j, "ratio": r,
                      "verdict": verdict_for(r), "pass": "fuzzy",
                      "raw_equal": A[i]["raw_hash"] == B[j]["raw_hash"]})
    matchedA = {p["i"] for p in pairs}
    matchedB = {p["j"] for p in pairs}
    unmA = [i for i in range(len(A)) if i not in matchedA]
    unmB = [j for j in range(len(B)) if j not in matchedB]
    stats = {
        "exact_pairs": sum(1 for p in pairs if p["verdict"] == EXACT),
        "exact_raw_equal": exact_raw,
        "fuzzy_pairs": sum(1 for p in pairs if p["pass"] == "fuzzy"),
        "pairs": len(pairs), "unmatched_orig": len(unmA), "unmatched_new": len(unmB),
        "candidate_pairs": len(candidates),
    }
    return pairs, unmA, unmB, stats


# ---------------------------------------------------------------------------
# self-test
# ---------------------------------------------------------------------------

def self_test(A, B, pairs, orig=None, new=None):
    """Assert the harness's own contracts.  Returns (passed, total, notes)."""
    passed = total = 0
    notes = []

    def check(cond, msg):
        nonlocal passed, total
        total += 1
        if cond:
            passed += 1
        else:
            notes.append("FAIL: " + msg)

    # 1. normalisation canonicalises addresses / branches
    mkb = lambda addr, mn, ops, size=4: {
        "addr": addr, "size": size, "mn": mn, "ops": ops, "text": f"{mn} {ops}",
        "comment": None, "branch": None, "refs": set()}
    f = {"addr": 0x1000, "end": 0x1100,
         "insns": [mkb(0x1000, "lea", "rcx,[rip+0x2e972]"),
                   mkb(0x1004, "mov", "eax,0x18005cbe8"),
                   mkb(0x1009, "jne", "0x1004", 2),
                   mkb(0x100b, "call", "0x180099999", 5)]}
    toks, raw, _ = normalise_function(f, 0x180000000, 0x180100000)
    check(toks[0] == "lea rcx,[rip+EXT]", f"rip canonicalisation: {toks[0]!r}")
    check(toks[1] == "mov eax,IMM_ADDR", f"imm canonicalisation: {toks[1]!r}")
    check(toks[2] == "jne L+0x4", f"internal branch label: {toks[2]!r}")
    check(toks[3] == "call EXT", f"external call: {toks[3]!r}")
    check(len(raw) == 4, "raw kept")

    # 2. trailing padding stripped
    g = {"addr": 0x2000, "end": 0x2010,
         "insns": [mkb(0x2000, "ret", "", 1), mkb(0x2001, "nop", "", 1),
                   mkb(0x2002, "nop", "", 1)]}
    tg, _, _ = normalise_function(g, 0x180000000, 0x180100000)
    check(tg == ["ret"], f"padding stripped: {tg!r}")

    # 3. verdict thresholds
    check(verdict_for(1.0, True) == EXACT, "exact verdict")
    check(verdict_for(0.95) == CLOSE, "close verdict")
    check(verdict_for(0.7) == SIMILAR, "similar verdict")
    check(verdict_for(0.5) == DIFFERENT, "different verdict")

    # 4. fuzzy pairing injects a known close pair
    def mkf(addr, tokens):
        return {"addr": addr, "end": addr + len(tokens), "size": len(tokens),
                "tokens": tokens, "raw_tokens": tokens, "icount": len(tokens),
                "mnem": Counter(t.split(" ", 1)[0] for t in tokens),
                "strict_hash": sha1("\n".join(tokens)),
                "raw_hash": sha1("\n".join(tokens)), "insns": [], "names": []}
    base = ["push rbp", "mov rbp,rsp", "mov eax,0x1", "mov rcx,0x2",
            "add eax,ecx", "pop rbp", "ret"]
    like = base[:3] + ["mov rcx,0x3"] + base[4:]
    unrel = ["mov rax,0x1", "mov rbx,0x2", "sub rax,rbx", "imul rax,rbx",
             "xor rdx,rdx", "ret"]
    X = [mkf(0x10, base), mkf(0x20, unrel)]
    Y = [mkf(0x30, like), mkf(0x40, unrel)]
    p, ua, ub, st = pair_functions(X, Y)
    got = {(X[q["i"]]["addr"], Y[q["j"]]["addr"]): q["verdict"] for q in p}
    check(got.get((0x20, 0x40)) == EXACT, f"injected exact pair: {got}")
    check(got.get((0x10, 0x30)) in (CLOSE, SIMILAR),
          f"injected close pair: {got}")

    # 5. normalisation vs byte equality.
    #    Contract that is actually true: two functions whose trailing-padding-
    #    stripped normalised *raw* text is identical have identical code bytes.
    #    Byte inequality of the full byte_str is expected and correct when the
    #    only difference is trailing nop/int3 padding.  Symmetrically,
    #    normalised-EXACT pairs (equal canonical tokens) need NOT be
    #    byte-identical: RIP-relative displacements and relocations encode
    #    different absolute addresses for the same canonical token stream.
    raw_equal_pairs = [q for q in pairs if q.get("raw_equal")]
    check(all(q["verdict"] == EXACT for q in raw_equal_pairs),
          "every raw-text-identical pair is verdict EXACT")
    check(len(raw_equal_pairs) >= 1, "at least one raw-text-identical pair matched")
    code_equal = all(A[q["i"]]["code_bytes"] == B[q["j"]]["code_bytes"]
                     for q in raw_equal_pairs)
    check(code_equal,
          "raw-text-identical pairs are byte-identical after trailing-padding strip")
    pad_only = [q for q in raw_equal_pairs
                if A[q["i"]]["byte_str"] != B[q["j"]]["byte_str"]
                and A[q["i"]]["code_bytes"] == B[q["j"]]["code_bytes"]]
    # independent byte-level pool: padding-stripped code bytes only
    code_pools = defaultdict(list)
    for j, b in enumerate(B):
        if b["code_bytes"]:
            code_pools[b["code_bytes"]].append(j)
    byte_identical_pairs = []
    for i, a in enumerate(A):
        if not a["code_bytes"]:
            continue
        for j in code_pools.get(a["code_bytes"], ()):
            byte_identical_pairs.append((i, j))
            if len(byte_identical_pairs) >= 5000:
                break
        if len(byte_identical_pairs) >= 5000:
            break
    pair_map = {(q["i"], q["j"]): q for q in pairs}
    chosen = [(i, j) for (i, j) in byte_identical_pairs if (i, j) in pair_map]
    check(all(pair_map[(i, j)]["verdict"] == EXACT for i, j in chosen),
          "every chosen code-byte-identical candidate pair is verdict EXACT")
    check(any(pair_map[(i, j)]["verdict"] == EXACT for i, j in chosen),
          "at least one code-byte-identical candidate pair was chosen and is EXACT")
    # normalised-EXACT is a token-level claim, not a byte-level claim: record
    # (do not assert) how many EXACT pairs still differ in code bytes because
    # toolchain/relocation encoding differs.
    exact_pairs = [q for q in pairs if q["verdict"] == EXACT]
    exact_byte_diff = [q for q in exact_pairs
                       if A[q["i"]]["code_bytes"] != B[q["j"]]["code_bytes"]]
    check(len(exact_pairs) >= len(raw_equal_pairs),
          "raw-text-identical pairs are a subset of normalised-EXACT pairs")
    notes.append(f"raw-text-identical EXACT pairs={len(raw_equal_pairs)} "
                 f"(trailing-padding-only byte diffs={len(pad_only)}); "
                 f"padding-stripped byte-identical candidate pairs={len(byte_identical_pairs)}; "
                 f"normalised-EXACT pairs={len(exact_pairs)} of which "
                 f"code-byte-different (relocation/toolchain)={len(exact_byte_diff)}")
    notes.append(f"self-test passed {passed}/{total}")
    return passed, total, notes


# ---------------------------------------------------------------------------
# report
# ---------------------------------------------------------------------------

def tier_counts(funcs, verdict_map, side):
    counts = defaultdict(lambda: Counter())
    for f in funcs:
        v = verdict_map.get(f["addr"], MISSING_ORIG if side == "new" else MISSING_NEW)
        counts[f["tier"]][v] += 1
    return counts


def write_report(path, orig, new, meta_o, meta_n, A, B, pairs, unmA, unmB, stats,
                 self_notes):
    O = {f["addr"]: f for f in A}
    N = {f["addr"]: f for f in B}
    o2n = {p["i"]: p for p in pairs}
    n2o = {p["j"]: p for p in pairs}
    o_pair_addr = {A[p["i"]]["addr"]: p for p in pairs}
    n_pair_addr = {B[p["j"]]["addr"]: p for p in pairs}
    L = []
    A_ = L.append
    A_("# Disassembly-Parity Baseline — SexLabPPrism.dll vs rebuild v1")
    A_("")
    A_("Generated by `tools/parity.py`. Both DLLs are read-only inputs.")
    A_("")
    A_("## 0. Harness self-test")
    A_("")
    for n in self_notes:
        A_(f"- {n}")
    A_("")

    A_("## 1. Binary metadata")
    A_("")
    A_("| field | original | rebuild |")
    A_("|---|---|---|")
    A_(f"| path | `{orig.path}` | `{new.path}` |")
    A_(f"| file bytes | {len(orig.data):,} | {len(new.data):,} |")
    A_(f"| linker version | {orig.linker[0]}.{orig.linker[1]} | {new.linker[0]}.{new.linker[1]} |")
    A_(f"| entry RVA | 0x{orig.entry:x} | 0x{new.entry:x} |")
    A_(f"| image base | 0x{orig.image_base:x} | 0x{new.image_base:x} |")
    A_(f"| size of image | 0x{orig.size_of_image:x} | 0x{new.size_of_image:x} |")
    A_(f"| subsystem ver | {orig.subsystem_ver[0]}.{orig.subsystem_ver[1]} | {new.subsystem_ver[0]}.{new.subsystem_ver[1]} |")
    A_(f"| checksum | 0x{orig.checksum:08x} | 0x{new.checksum:08x} |")
    A_(f"| .pdata entries | {meta_o['pdata_entries']} | {meta_n['pdata_entries']} |")
    A_(f"| .pdata coverage of .text | {meta_o['pdata_coverage_frac']*100:.2f}% ({meta_o['pdata_covered_bytes']:,}/{meta_o['text_vsize']:,} B) | {meta_n['pdata_coverage_frac']*100:.2f}% ({meta_n['pdata_covered_bytes']:,}/{meta_n['text_vsize']:,} B) |")
    A_(f"| uncovered .text bytes | {meta_o['uncovered_bytes']:,} (code {meta_o['uncovered_code_bytes']:,}, pad {meta_o['uncovered_padding_bytes']:,}) | {meta_n['uncovered_bytes']:,} (code {meta_n['uncovered_code_bytes']:,}, pad {meta_n['uncovered_padding_bytes']:,}) |")
    A_(f"| gap (no-unwind) functions | {meta_o['gap_functions']} | {meta_n['gap_functions']} |")
    A_("")
    A_("### Sections")
    A_("")
    A_("| section | orig VA | orig vsize | orig raw | new VA | new vsize | new raw |")
    A_("|---|---|---|---|---|---|---|")
    for name in sorted({s["name"] for s in orig.sections} | {s["name"] for s in new.sections}):
        so = next((s for s in orig.sections if s["name"] == name), None)
        sn = next((s for s in new.sections if s["name"] == name), None)
        fmt = lambda s: (f"0x{s['va']:x}", f"{s['vsize']:,}", f"{s['rsize']:,}") if s else ("-", "-", "-")
        a = fmt(so); b = fmt(sn)
        A_(f"| {name} | {a[0]} | {a[1]} | {a[2]} | {b[0]} | {b[1]} | {b[2]} |")
    A_("")
    A_("### Rich header (MSVC toolset fingerprint, parses CORRECTLY here)")
    A_("")
    for label, pe in (("original", orig), ("rebuild", new)):
        if not pe.rich:
            A_(f"- **{label}**: no Rich header found")
            continue
        A_(f"- **{label}**: key=0x{pe.rich['key']:08x}, {len(pe.rich['entries'])} entries")
        A_("")
        A_("  | comp_id | productId | product | build | count |")
        A_("  |---|---|---|---|---|")
        for comp, count in pe.rich["entries"]:
            pid = comp >> 16
            build = comp & 0xFFFF
            A_(f"  | 0x{comp:08x} | 0x{pid:04x} | {RICH_PRODUCTS.get(pid, '?')} | {build} | {count} |")
    A_("")
    A_("### Original `.rsrc` VERSIONINFO")
    A_("")
    if orig.version and orig.version.get("types"):
        for t in orig.version["types"]:
            for k, v in t["blocks"]["fixed"].items():
                A_(f"- fixed `{k}` = `{v}`")
            A_("")
            for k, v in t["blocks"]["strings"].items():
                A_(f"- string `{k}` = `{v}`")
    else:
        A_("- (no VERSIONINFO resource parsed)")
    if new.version and new.version.get("types"):
        A_("")
        A_("Rebuild VERSIONINFO: " + ", ".join(
            f"{k}={v}" for t in new.version["types"] for k, v in t["blocks"]["strings"].items()))
    A_("")

    # verdicts
    def verdict_of(addr, side):
        if side == "orig":
            p = o_pair_addr.get(addr)
            return p["verdict"] if p else MISSING_NEW
        p = n_pair_addr.get(addr)
        return p["verdict"] if p else MISSING_ORIG

    order = [EXACT, CLOSE, SIMILAR, DIFFERENT, MISSING_NEW, MISSING_ORIG]
    A_("## 2. Per-tier verdict counts")
    A_("")
    A_("Rule used: **library-inlined** if the best-effort name matches an STL / spdlog / "
       "CommonLibSSE marker; otherwise **plugin-owned** if it is an export, references a "
       "plugin string/RTTI descriptor, matches a known plugin function name, or is reachable "
       "by direct call from those entrypoints (without descending into library-named code). "
       "`basis` fields in `per-function.json` record which rule fired.")
    A_("")
    for tier in ("plugin", "library"):
        A_(f"### tier = {tier}")
        A_("")
        oc = Counter(verdict_of(f["addr"], "orig") for f in A if f["tier"] == tier)
        nc = Counter(verdict_of(f["addr"], "new") for f in B if f["tier"] == tier)
        A_(f"- original functions: {sum(oc.values())}  " +
           ", ".join(f"{v}={oc.get(v,0)}" for v in order if oc.get(v)))
        A_(f"- rebuild functions:  {sum(nc.values())}  " +
           ", ".join(f"{v}={nc.get(v,0)}" for v in order if nc.get(v)))
        A_("")
    all_oc = Counter(verdict_of(f["addr"], "orig") for f in A)
    all_nc = Counter(verdict_of(f["addr"], "new") for f in B)
    A_(f"**Totals** — original {len(A)}: " + ", ".join(f"{v}={all_oc.get(v,0)}" for v in order) +
       f"  |  rebuild {len(B)}: " + ", ".join(f"{v}={all_nc.get(v,0)}" for v in order))
    A_(f"**Pair passes** — exact pairs {stats['exact_pairs']} (raw-text-identical {stats['exact_raw_equal']}), "
       f"fuzzy pairs {stats['fuzzy_pairs']}, leftover orig {stats['unmatched_orig']}, leftover new {stats['unmatched_new']}.")
    A_("")

    # plugin-owned named table
    A_("## 3. Plugin-owned functions we can name")
    A_("")
    A_("| name | orig addr | tier | verdict | ratio | orig insn | new insn | new addr |")
    A_("|---|---|---|---|---|---|---|---|")
    ai = {f["addr"]: i for i, f in enumerate(A)}
    known = [f for f in A if f["tier"] == "plugin" and f["name"]]
    known.sort(key=lambda f: (f["addr"]))
    for f in known:
        p = o2n.get(ai[f["addr"]])
        v = verdict_of(f["addr"], "orig")
        nm = (f["name"] or "")[:58].replace("|", "/")
        if p:
            nf = B[p["j"]]
            A_(f"| {nm} | 0x{f['addr']:x} | {f['tier']} | {v} | {p['ratio']:.3f} | "
               f"{f['icount']} | {nf['icount']} | 0x{nf['addr']:x} |")
        else:
            A_(f"| {nm} | 0x{f['addr']:x} | {f['tier']} | {v} | - | {f['icount']} | - | - |")
    A_("")

    # all named original functions (the __FUNCSIG__ / functions.json set)
    A_("## 3b. Every named original function (__FUNCSIG__ / recon set)")
    A_("")
    A_("| name | orig addr | tier | verdict | ratio | orig insn | new addr |")
    A_("|---|---|---|---|---|---|---|")
    for f in A:
        if not f["name"]:
            continue
        p = o2n.get(ai[f["addr"]])
        v = verdict_of(f["addr"], "orig")
        nm = (f["name"] or "")[:58].replace("|", "/")
        na = hex(B[p["j"]]["addr"]) if p else "-"
        A_(f"| {nm} | 0x{f['addr']:x} | {f['tier']} | {v} | "
           f"{p['ratio']:.3f} | {f['icount']} | {na} |" if p else
           f"| {nm} | 0x{f['addr']:x} | {f['tier']} | {v} | - | {f['icount']} | - |")
    A_("")

    # best / worst
    fuzzy = sorted(pairs, key=lambda p: -p["ratio"])
    worst = sorted(pairs, key=lambda p: p["ratio"])
    A_("## 4. Best / worst pairs")
    A_("")
    A_("### 15 highest-ratio pairs below EXACT (where to confirm reconstruction)")
    A_("")
    A_("| ratio | verdict | orig addr | orig name | new addr | orig/new insn |")
    A_("|---|---|---|---|---|---|")
    for p in [x for x in fuzzy if x["verdict"] != EXACT][:15]:
        f, nf = A[p["i"]], B[p["j"]]
        A_(f"| {p['ratio']:.3f} | {p['verdict']} | 0x{f['addr']:x} | {(f['name'] or '')[:45]} | "
           f"0x{nf['addr']:x} | {f['icount']}/{nf['icount']} |")
    A_("")
    A_("### 15 lowest-ratio pairs (worst divergence; included only when the tight signature filter held)")
    A_("")
    A_("| ratio | verdict | orig addr | orig name | new addr | orig/new insn |")
    A_("|---|---|---|---|---|---|")
    for p in worst[:15]:
        f, nf = A[p["i"]], B[p["j"]]
        A_(f"| {p['ratio']:.3f} | {p['verdict']} | 0x{f['addr']:x} | {(f['name'] or '')[:45]} | "
           f"0x{nf['addr']:x} | {f['icount']}/{nf['icount']} |")
    A_("")

    # toolchain delta
    A_("## 5. Toolchain delta")
    A_("")
    A_(f"- original linker {orig.linker[0]}.{orig.linker[1]}, rebuild linker {new.linker[0]}.{new.linker[1]}.")
    A_("- Rich header comp IDs above show the exact build numbers; differences in the "
       "`Utc*` / compiler product build are the MSVC toolset delta. A different compiler "
       "build changes register allocation, inlining decisions, instruction selection "
       "(e.g. `lea` vs `mov`+`add`), NOP padding, and STL inlining — so even byte-correct C++ "
       "source can yield a different normalised token stream for the same function.")
    A_("")

    # limitations
    A_("## 6. What this harness cannot see")
    A_("")
    A_("- **.pdata coverage gap.** Functions without unwind info (leafs) are recovered by a "
       "gap scan with heuristics; adjacent leaves with no padding can be merged or split. "
       f"Coverage: original {meta_o['pdata_coverage_frac']*100:.2f}%, rebuild {meta_n['pdata_coverage_frac']*100:.2f}% of .text.")
    A_("- **Inlined code.** Source-level changes that move work across function boundaries "
       "(inlining/outlining) change function counts and instruction counts; the harness sees "
       "the compiled result only.")
    A_("- **ICF (identical code folding).** The linker merges identical functions; a folded "
       "thunk may appear once in one binary and multiple times in the other, so exact-hash "
       "matching can mispair or leave duplicates unmatched.")
    A_("- **Compiler merge/split.** Tail-merge, hot/cold splitting and function layout differ "
       "between toolset builds; a single source function can appear as several `.pdata` ranges.")
    A_("- **No symbols.** Names are best-effort: `__FUNCSIG__` literals embedded by spdlog "
       "plus prior recon JSON; unnamed functions have no name and cannot be matched by name.")
    A_("- **Fuzzy floor.** Pairs below the tight-signature filter are deliberately left "
       "unmatched; `ratio` from SequenceMatcher is a token-order heuristic, not a semantic "
       "equivalence proof. Lowering `FUZZY_MIN` trades coverage for false pairings.")
    A_("- **Classification is heuristic.** Reachability-based plugin ownership can pull in "
       "unnamed STL/CRT helpers; name markers can miss unnamed std internals. `basis` is "
       "recorded per function so the split can be re-derived.")
    A_("- **No name-assisted pairing (deliberate).** Per the `no forced pairing` rule the "
       "harness runs exactly three passes (exact / structural / leftover). A rebuild function "
       "that corresponds by name but diverges beyond the structural tolerance is reported "
       "`MISSING-NEW` / `MISSING-ORIG`, not `DIFFERENT`. Concretely the rebuild's "
       "`Papyrus::Natives::Log` (0x180017bf0, 55 insn) and `Papyrus::Natives::Register` "
       "(0x180018a70, 657 insn) do not pair with the original `Papyrus_Log` (0x18002a8e0, "
       "37 insn) / registration lambda (0x180026a80, 555 insn) at the token level. "
       "This is exactly what `tools/parity_names.py` addresses: it recovers a real "
       "name -> function map on both sides (embedded `__FUNCSIG__` + MSVC RTTI) and "
       "pairs by name, with a per-pair divergence diff.")
    A_("")
    A_("## 7. Reproduce")
    A_("")
    A_("```")
    A_("python3 tools/parity.py            # writes this file + recon/parity/per-function.json")
    A_("python3 tools/pe-compare.py artifacts/SexLabPPrism.dll artifacts/rebuild/SexLabPPrism-rebuild-v1.dll")
    A_("```")
    open(path, "w").write("\n".join(L) + "\n")
    return path


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--orig", default="artifacts/SexLabPPrism.dll")
    ap.add_argument("--new", default="artifacts/rebuild/SexLabPPrism-rebuild-v1.dll")
    ap.add_argument("--outdir", default="recon/parity")
    ap.add_argument("--report", default="recon/parity-baseline.md")
    ap.add_argument("--names", default="recon/functions.json")
    ap.add_argument("--keep-asm", action="store_true")
    ap.add_argument("--self-test", action="store_true", default=True)
    args = ap.parse_args()

    os.makedirs(args.outdir, exist_ok=True)
    orig = PE(args.orig)
    new = PE(args.new)

    keep_o = os.path.join(args.outdir, "objdump-orig.asm") if args.keep_asm else None
    keep_n = os.path.join(args.outdir, "objdump-new.asm") if args.keep_asm else None
    ins_o = parse_objdump(run_objdump(args.orig, keep=keep_o), orig.image_base)
    ins_n = parse_objdump(run_objdump(args.new, keep=keep_n), new.image_base)

    fo, meta_o = build_functions(orig, ins_o)
    fn, meta_n = build_functions(new, ins_n)

    # names
    name_map, nj = load_name_json(args.names, orig)
    for f in fo:
        for n in name_map.get(f["addr"], []):
            f.setdefault("names", []).append(n)
    recover_sig_names(orig, fo)
    recover_sig_names(new, fn)

    # classification
    plugin_o = plugin_ref_addresses(orig)
    plugin_n = plugin_ref_addresses(new)
    known_o = load_manifest("recon/decompiled/MANIFEST.txt")
    classify(orig, fo, name_map, orig.exports(), plugin_o, known_o)
    classify(new, fn, {}, new.exports(), plugin_n, set())

    # comparison
    prepare(fo, orig.image_base, orig.size_of_image)
    prepare(fn, new.image_base, new.size_of_image)
    pairs, unmA, unmB, stats = pair_functions(fo, fn)

    if args.self_test:
        passed, total, notes = self_test(fo, fn, pairs, orig, new)
    else:
        passed, total, notes = 0, 0, []

    # output per-function.json
    o2n = {p["i"]: p for p in pairs}
    n2o = {p["j"]: p for p in pairs}

    def rec_o(i):
        f = fo[i]
        p = o2n.get(i)
        r = {"side": "orig", "addr": hex(f["addr"]), "size": f["size"],
             "insn": f["icount"], "name": f["name"], "tier": f["tier"],
             "basis": f["basis"], "source": f["source"]}
        if p:
            nf = fn[p["j"]]
            r.update({"verdict": p["verdict"], "ratio": round(p["ratio"], 6),
                      "new_addr": hex(nf["addr"]), "new_size": nf["size"],
                      "new_insn": nf["icount"], "pass": p["pass"],
                      "raw_equal": p["raw_equal"]})
        else:
            r.update({"verdict": MISSING_NEW, "ratio": None, "new_addr": None})
        return r

    def rec_n(j):
        f = fn[j]
        p = n2o.get(j)
        r = {"side": "new", "addr": hex(f["addr"]), "size": f["size"],
             "insn": f["icount"], "name": f["name"], "tier": f["tier"],
             "basis": f["basis"], "source": f["source"]}
        if p:
            of = fo[p["i"]]
            r.update({"verdict": p["verdict"], "ratio": round(p["ratio"], 6),
                      "orig_addr": hex(of["addr"]), "orig_size": of["size"],
                      "orig_insn": of["icount"], "pass": p["pass"],
                      "raw_equal": p["raw_equal"]})
        else:
            r.update({"verdict": MISSING_ORIG, "ratio": None, "orig_addr": None})
        return r

    out = {
        "harness": "tools/parity.py",
        "orig": {"path": args.orig, "linker": list(orig.linker), "bytes": len(orig.data),
                 "image_base": hex(orig.image_base), "size_of_image": hex(orig.size_of_image),
                 "rich": orig.rich, "sections": orig.sections,
                 "exports": orig.exports(), "version": orig.version,
                 "pdata_entries": meta_o["pdata_entries"],
                 "pdata_coverage_frac": round(meta_o["pdata_coverage_frac"], 6),
                 "uncovered_bytes": meta_o["uncovered_bytes"],
                 "gap_functions": meta_o["gap_functions"]},
        "new": {"path": args.new, "linker": list(new.linker), "bytes": len(new.data),
                "image_base": hex(new.image_base), "size_of_image": hex(new.size_of_image),
                "rich": new.rich, "sections": new.sections,
                "exports": new.exports(), "version": new.version,
                "pdata_entries": meta_n["pdata_entries"],
                "pdata_coverage_frac": round(meta_n["pdata_coverage_frac"], 6),
                "uncovered_bytes": meta_n["uncovered_bytes"],
                "gap_functions": meta_n["gap_functions"]},
        "summary": stats,
        "self_test": {"passed": passed, "total": total, "notes": notes},
        "orig_functions": [rec_o(i) for i in range(len(fo))],
        "new_functions": [rec_n(j) for j in range(len(fn))],
        "pairs": [{"orig": hex(fo[p["i"]]["addr"]), "new": hex(fn[p["j"]]["addr"]),
                   "verdict": p["verdict"], "ratio": round(p["ratio"], 6),
                   "orig_insn": fo[p["i"]]["icount"], "new_insn": fn[p["j"]]["icount"],
                   "orig_tier": fo[p["i"]]["tier"], "new_tier": fn[p["j"]]["tier"],
                   "pass": p["pass"], "raw_equal": p["raw_equal"]}
                  for p in pairs],
        "unmatched_orig": [hex(fo[i]["addr"]) for i in unmA],
        "unmatched_new": [hex(fn[j]["addr"]) for j in unmB],
    }
    jpath = os.path.join(args.outdir, "per-function.json")
    json.dump(out, open(jpath, "w"), indent=1)

    rpath = write_report(args.report, orig, new, meta_o, meta_n, fo, fn, pairs,
                         unmA, unmB, stats, notes)

    # ---- compact stdout (< 40 lines) ----
    def counts(fs, side):
        c = Counter()
        for i, f in enumerate(fs):
            if side == "orig":
                c[o2n[i]["verdict"] if i in o2n else MISSING_NEW] += 1
            else:
                c[n2o[i]["verdict"] if i in n2o else MISSING_ORIG] += 1
        return c

    print(f"objdump insns: orig {len(ins_o)}, new {len(ins_n)}")
    print(f"functions: orig {len(fo)} (pdata {meta_o['pdata_entries']} + gap {meta_o['gap_functions']}), "
          f"new {len(fn)} (pdata {meta_n['pdata_entries']} + gap {meta_n['gap_functions']})")
    print(f".text pdata coverage: orig {meta_o['pdata_coverage_frac']*100:.2f}%, new {meta_n['pdata_coverage_frac']*100:.2f}%")
    print(f"pairing: exact {stats['exact_pairs']} (raw-text-equal {stats['exact_raw_equal']}), "
          f"fuzzy {stats['fuzzy_pairs']}, unmatched orig {stats['unmatched_orig']}, new {stats['unmatched_new']}")
    for tier in ("plugin", "library"):
        oc = Counter((o2n[i]["verdict"] if i in o2n else MISSING_NEW)
                     for i, f in enumerate(fo) if f["tier"] == tier)
        nc = Counter((n2o[j]["verdict"] if j in n2o else MISSING_ORIG)
                     for j, f in enumerate(fn) if f["tier"] == tier)
        print(f"  {tier:8s} orig {sum(oc.values()):4d}  " +
              " ".join(f"{k}={oc.get(k,0)}" for k in (EXACT, CLOSE, SIMILAR, DIFFERENT, MISSING_NEW)))
        print(f"  {'':8s} new  {sum(nc.values()):4d}  " +
              " ".join(f"{k}={nc.get(k,0)}" for k in (EXACT, CLOSE, SIMILAR, DIFFERENT, MISSING_ORIG)))
    print(f"self-test: {passed}/{total} passed")
    for n in notes:
        print("  " + n)
    print(f"wrote {jpath}")
    print(f"wrote {rpath}")


if __name__ == "__main__":
    main()