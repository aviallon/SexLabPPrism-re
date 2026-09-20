#!/usr/bin/env python3
"""Assert a built PE's toolset (linker version) and report its Rich header ids.

The original SexLabPPrism.dll was linked with MSVC 14.44. A parity build is
worthless if the runner silently falls back to a newer toolset, so this is a
hard gate rather than a log line.

Usage: check_linker.py <dll> <expected-major>.<expected-minor>
Exit 0 = the linker version matches; exit 1 = it does not.
"""
import pathlib
import struct
import sys


def u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def rich_ids(data):
    """Return [(productId, build, count)] from the Rich header.

    Layout: DanS | 3 pad dwords | (comp_id, count) pairs ... | Rich | key,
    every dword XORed with the key; comp_id = (productId << 16) | buildNumber.
    """
    ridx = data.find(b"Rich", 0x40, 0x4000)
    if ridx < 0:
        return []
    key = u32(data, ridx + 4)
    dans = None
    p = ridx - 8
    while p > 0x3F:
        if u32(data, p) ^ key == 0x536E6144:  # 'DanS'
            dans = p
            break
        p -= 8
    if dans is None:
        return []
    ids, q = [], dans + 16  # skip DanS + 3 padding dwords
    while q + 7 < ridx:
        comp = u32(data, q) ^ key
        count = u32(data, q + 4) ^ key
        ids.append((comp >> 16, comp & 0xFFFF, count))
        q += 8
    return ids


dll = pathlib.Path(sys.argv[1])
want = sys.argv[2] if len(sys.argv) > 2 else "14.44"
data = dll.read_bytes()
pe = u32(data, 0x3C)
opt = pe + 24
major, minor = data[opt + 2], data[opt + 3]
got = f"{major}.{minor}"
ids = rich_ids(data)
builds = sorted({b for _, b, _ in ids})
products = sorted({p for p, _, _ in ids})

print(f"linker version of {dll.name}: {got}")
print(f"Rich header: {len(ids)} entries, products {products}, builds {builds}")
if got != want:
    print(f"::error::expected linker {want} but the binary was linked with {got} - "
          f"the toolset selection did not take effect", file=sys.stderr)
    sys.exit(1)
print(f"OK: linker {got} matches the original toolset")