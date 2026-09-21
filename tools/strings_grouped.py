#!/usr/bin/env python3
"""Grouped string table for SexLabPPrism.dll -> recon/strings.txt

Two passes: ASCII (>=4 chars) and UTF-16LE.  Mangled C++ names, raw
instruction bytes and CRT symbol blobs are excluded from the human
sections; they are summarised by count instead.
"""
import re, collections

PATH = "artifacts/SexLabPPrism.dll"
data = open(PATH, "rb").read()

ascii_strs = sorted(set(m.group().decode("latin1") for m in
                        re.finditer(rb"[\x20-\x7e]{4,}", data)))
utf16_strs = sorted(set(m.group().decode("utf-16le", "replace") for m in
                        re.finditer(rb"(?:[\x20-\x7e]\x00){4,}", data)))

MANGLE = re.compile(r"@@|__cdecl|__fastcall|anonymous-namespace|basic_string|"
                    r"char_traits|allocator|rapidcsv|spdlog|std::|"
                    r"^[A-Za-z_]*@[A-Za-z]|libcpmt|api-ms-win|"
                    r"[A-Za-z0-9/_]*\.(dll|exe)$")
JUNK = re.compile(r"^[\$0-9A-Fa-f\\\[\(\)\^_\|;+=!@#%.<>&~*\- ]{4,}$")
def is_junk(s):
    if JUNK.match(s):
        return True
    # instruction-byte soup: many high chars + escapes
    non = sum(1 for c in s if not (32 <= ord(c) < 127))
    return non > 0

def human(s):
    if len(s) < 4 or MANGLE.search(s):
        return False
    if s.startswith("?") or s.startswith("."):
        return False
    if re.match(r"^(KERNEL32|USER32|SHELL32|ole32|VERSION|MSVCP|VCRUNTIME)", s):
        return False
    # names of imported functions are in imports.txt; here keep strings with a space
    # or that look like identifiers used by the plugin
    return True

def has_fmt(s):
    return bool(re.search(r"\{[^}]*\}|%[-#0-9.]*[sdufgxX]|\{0\}", s))

def is_path(s):
    return bool(re.search(r"[\\/]", s) and re.search(r"\.(bin|csv|log|html|dll|exe|h|cpp|json|pei?x)\b", s))

# ---- curated sets -------------------------------------------------------
funcsig = [s for s in ascii_strs if "__cdecl" in s or "__fastcall" in s]
buildpaths = [s for s in ascii_strs if "commonlibsse-ng" in s or re.match(r"^src[\\/]", s)]
msgs = [s for s in ascii_strs
        if human(s) and " " in s and len(s) > 12
        and not has_fmt(s) and not s.startswith("???")]
msgs_fmt = [s for s in ascii_strs
            if human(s) and " " in s and len(s) > 12 and has_fmt(s)]
paths = sorted(set(s for s in ascii_strs if is_path(s) and human(s)))
paths += sorted(set(s for s in utf16_strs if is_path(s) and human(s)))
papyrus = [s for s in ascii_strs if re.search(
    r"Papyrus_|BeginSceneSession|PublishSceneState|PublishCompatible|"
    r"SetSearchQuery|CatalogBegin|CatalogFinish|CatalogPackage|CatalogPublish|"
    r"GetCatalogCount|IsCatalogReady|IsFreeCameraActive", s)
    and not MANGLE.search(s)]
ui = [s for s in ascii_strs if re.match(r"^(slpp|SLPP)", s)]
ui += [s for s in ascii_strs if re.search(r"Prisma|UI action|UI mode|view |View", s)
       and human(s) and (" " in s or s.startswith("slpp") or s.startswith("SLPP"))]
jsontpl = [s for s in ascii_strs if s.startswith('{') or s.startswith('{{')]
rel = [s for s in ascii_strs if re.search(
    r"address library|Address Library|versionlib|version-\{|Offsets-v2|"
    r"IDDatabase|REL::|runtime|Runtime|VR\b", s) and human(s)]
crthex = sorted(set(s for s in ascii_strs if re.match(
    r"^(Get|Set|Create|Close|Find|Map|Unmap|Write|Read|Flush|Delete|"
    r"Initialize|Is|GetProc|Disable|Acquire|Release|Query|Format)", s)
    and len(s) > 5 and s.isascii()))
vs = sorted(set(s for s in utf16_strs if s and not is_junk(s) and len(s) >= 4))

out = []
A = out.append
A("STRING TABLE for %s" % PATH)
A("ASCII strings >=4 chars: %d    UTF-16LE strings >=4 chars: %d" % (len(ascii_strs), len(utf16_strs)))
A("Convention: [FMT] = contains a fmt/printf placeholder, [PATH] = filesystem path.")
A("")
A("#" * 72)
A("# 1. BUILD / VERSION PROVENANCE")
A("#" * 72)
for s in sorted(set(utf16_strs)):
    if re.match(r"^(\d+\.\d+\.\d+\.\d+|SexLabPPrism|PrismaUI scene|SickBaddie|CommonLibSSEOffsets|SKSE_RUNTIME|SkyrimSE\.exe|SkyrimVR\.exe|040904b0|VS_VERSION|StringFileInfo|VarFileInfo|Translation|FileVersion|ProductVersion|ProductName|InternalName|FileDescription|LegalCopyright|CompanyName)", s):
        A("  %s" % s)
A("")
A("#" * 72)
A("# 2. SOURCE PATHS + __FUNCSIG__ (real function names/signatures)  [%d funcsigs]" % len(funcsig))
A("#" * 72)
A("Embedded source paths (compiler source_location):")
for s in sorted(set(buildpaths)):
    A("  [PATH] %s" % s)
A("")
A("__FUNCSIG__ literals (see recon/functions.txt for address mapping):")
for s in sorted(set(funcsig)):
    A("  %s" % s)
A("")
A("#" * 72)
A("# 3. PAPYRUS NATIVE API (names + registration signatures)")
A("#" * 72)
for s in sorted(set(papyrus)):
    A("  [FMT] %s" % s if has_fmt(s) else "  %s" % s)
A("")
A("#" * 72)
A("# 4. UI / JS BRIDGE / JSON TEMPLATES / VIEW KEYS")
A("#" * 72)
for s in sorted(set(jsontpl)):
    A("  [FMT] %s" % s)
for s in sorted(set(ui)):
    A("  %s" % s)
A("")
A("#" * 72)
A("# 5. LOG / DIAGNOSTIC MESSAGES (spdlog)")
A("#" * 72)
A("-- messages with format placeholders --")
for s in sorted(set(msgs_fmt)):
    A("  [FMT] %s" % s)
A("-- fixed messages --")
for s in sorted(set(msgs)):
    A("  %s" % s)
A("")
A("#" * 72)
A("# 6. CONFIG / FILE PATHS")
A("#" * 72)
for s in sorted(set(paths)):
    tag = " [FMT]" if has_fmt(s) else ""
    A("  %s%s" % (s, tag))
A("")
A("#" * 72)
A("# 7. REL / ADDRESS-LIBRARY MACHINERY")
A("#" * 72)
for s in sorted(set(rel)):
    tag = " [FMT]" if has_fmt(s) else ""
    A("  %s%s" % (s, tag))
A("")
A("#" * 72)
A("# 8. UTF-16 RESOURCE STRINGS (VS_VERSION_INFO etc.)  [%d]" % len(vs))
A("#" * 72)
for s in vs:
    A("  %s" % s)
A("")
A("#" * 72)
A("# 9. WINDOWS / CRT SYMBOL NAMES SEEN IN THE IMAGE (imports are in recon/imports.txt)")
A("#" * 72)
for s in crthex:
    A("  %s" % s)
A("")
A("#" * 72)
A("# 10. FORMAT-SPECIFIER INVENTORY")
A("#" * 72)
fmts = sorted(set(re.findall(r"\{[^}\n]{0,20}\}", " ".join(ascii_strs))))
A("fmt-style placeholders: %s" % " ".join(fmts))
printf = sorted(set(re.findall(r"%[-#0-9.]*[sdufgxXc]", " ".join(ascii_strs))))
A("printf-style: %s" % " ".join(printf))
A("")
A("#" * 72)
A("# EXCLUDED: mangled C++ symbol strings and raw instruction bytes.")
A("# Mangled/symbol-like ASCII strings excluded: %d" %
  sum(1 for s in ascii_strs if MANGLE.search(s)))

open("recon/strings.txt", "w").write("\n".join(out) + "\n")
print("wrote recon/strings.txt: %d lines; ascii=%d utf16=%d funcsig=%d msgs=%d"
      % (len(out), len(ascii_strs), len(utf16_strs), len(funcsig), len(msgs)))