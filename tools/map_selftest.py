#!/usr/bin/env python3
"""Self-test for the MSVC /MAP symbol resolver used by ``match.py --map``.

The resolver must bind a declaration ONLY on an exact symbol match.  The
negative cases required by the round-13 brief are:

  * the symbol is absent from the map            -> "symbol not found"
  * the symbol resolves OUTSIDE the .text range  -> "address outside .text"
  * the symbol appears at two distinct addresses -> "duplicate symbol"

It also checks the exact-boundary rule (``?FooExtra@`` must not satisfy ``Foo``)
and that a C-linkage name and an anonymous-namespace mangled name both resolve.

Usage:  python3 tools/map_selftest.py
"""
from __future__ import annotations

import os
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import match as M  # noqa: E402

# A minimal but realistic /MAP body.  .text is declared as
# 0x180001000 .. 0x180002000 for the test.
SAMPLE_MAP = """\
 SexLabPPrism

 Timestamp is ... (placeholder)

  Preferred load address is 0000000180000000

  Start         Length     Name                   Class
  0001:0000000000001000 00001000H .text                   CODE
  0002:0000000000002000 00000a00H .rdata                  DATA

  Address         Publics by Value              Rva+Base       Lib:Object

 0001:0000000000001000       ?GoodFunc@@YAXXZ             0000000180001000     f   foo.obj
 0001:0000000000001100       ?Anon@?A0xdeadbeef@@YAXXZ    0000000180001100     f   bar.obj
 0001:0000000000001200       ?FooExtra@?A0xdeadbeef@@YAXXZ 0000000180001200    f   bar.obj
 0001:0000000000001300       ?DupFunc@@YAXXZ              0000000180001300     f   foo.obj
 0001:0000000000001400       ?DupFunc@@YAXXZ              0000000180001400     f   bar.obj
 0001:0000000000001500       extern_c_fn                  0000000180001500     f   foo.obj

 Static symbols

 0001:0000000000001600       ?StaticThing@?A0xdeadbeef@@YAXXZ 0000000180001600  f  bar.obj

 0002:0000000000002000       ?OutsideFunc@@YAXXZ          0000000180002000     f   foo.obj
"""

TEXT_LO, TEXT_HI = 0x180001000, 0x180002000


def main():
    passed = total = 0
    notes = []

    def check(cond, msg):
        nonlocal passed, total
        total += 1
        passed += bool(cond)
        notes.append(("PASS " if cond else "FAIL ") + msg)

    with tempfile.TemporaryDirectory() as td:
        p = os.path.join(td, "SexLabPPrism.map")
        open(p, "w").write(SAMPLE_MAP)
        syms = M.load_msvc_map(p)

    def res(name, lo=TEXT_LO, hi=TEXT_HI):
        return M.resolve_map_symbol(syms, name, lo, hi)

    # parsing recovered every symbol at its Rva+Base
    check(syms.get("?GoodFunc@@YAXXZ") == {0x180001000},
          f"parse: GoodFunc -> {syms.get('?GoodFunc@@YAXXZ')}")
    check(syms.get("?DupFunc@@YAXXZ") == {0x180001300, 0x180001400},
          f"parse: DupFunc has 2 addresses -> {syms.get('?DupFunc@@YAXXZ')}")
    check(syms.get("extern_c_fn") == {0x180001500},
          f"parse: C symbol -> {syms.get('extern_c_fn')}")

    # positives: global C++ name, anonymous-namespace name, static symbol, C name
    check(res("GoodFunc") == (0x180001000, ""),
          f"exact: GoodFunc -> {res('GoodFunc')}")
    check(res("Anon") == (0x180001100, ""),
          f"exact: anonymous-ns Anon -> {res('Anon')}")
    check(res("StaticThing") == (0x180001600, ""),
          f"exact: static symbol -> {res('StaticThing')}")
    check(res("extern_c_fn") == (0x180001500, ""),
          f"exact: C-linkage name -> {res('extern_c_fn')}")

    # A. missing symbol
    got = res("NoSuchSymbol")
    check(got == (None, "symbol not found"), f"A. missing -> {got}")

    # B. symbol present but address outside .text
    got = res("OutsideFunc")
    check(got == (None, "address outside .text"), f"B. outside .text -> {got}")

    # C. duplicate symbol at two distinct addresses
    got = res("DupFunc")
    check(got == (None, "duplicate symbol (2 addresses)"),
          f"C. duplicate -> {got}")

    # D. exact-boundary: ?FooExtra@ must NOT satisfy Foo
    got = res("Foo")
    check(got == (None, "symbol not found"),
          f"D. boundary (?FooExtra@ vs Foo) -> {got}")

    # E. a text range that excludes a valid symbol still rejects it
    got = res("GoodFunc", lo=0x180002000, hi=0x180003000)
    check(got == (None, "address outside .text"),
          f"E. shifted .text range -> {got}")

    # F. the declared-symbol extraction used for PRESENT-UNPAIRED rows
    dec = {"our_symbol_or_source_location": "src/Presentation.cpp:143",
           "evidence": "src/missing/MissingRound2Group1.cpp:11 claims "
                       "`anonymous-namespace'::ApplyPresentation -> "
                       "src/Presentation.cpp:143"}
    check(M.declared_map_symbol(dec) == "ApplyPresentation",
          f"F. evidence symbol -> {M.declared_map_symbol(dec)!r}")
    dec = {"our_symbol_or_source_location": "ShutdownCaptionStore",
           "evidence": "src/missing/MissingRound2Group1.cpp:66"}
    check(M.declared_map_symbol(dec) == "ShutdownCaptionStore",
          f"G. reconstructed symbol -> {M.declared_map_symbol(dec)!r}")

    # H. apply_declared's map branch: binds, and rejects with a reason.  Tiny
    # synthetic sides so no DLL is needed; verdicts() only reads addr/tokens/
    # strict_hash/byte_hash.
    fo = [{"addr": 0x180001000, "tokens": ["a", "b", "c"], "name": "",
           "strict_hash": "o", "byte_hash": "ob"}]
    fn = [{"addr": 0x180001100, "tokens": ["a", "b", "d"],
           "strict_hash": "n", "byte_hash": "nb"}]
    decl = {0x180001000: {"orig_addr": "0x180001000",
                          "our_symbol_or_source_location": "Foo",
                          "evidence": "selftest:H", "kind": "RECONSTRUCTED"}}

    def run(resolver):
        return M.apply_declared(fo, fn, [], [0], [0], dict(decl), 0.5,
                                resolver)[3][0]

    row = run(lambda n: (0x180001100, ""))
    check(row["binding"] == "map" and row["verdict"] == M.DECLARED
          and row["bytematch"] == 0,
          f"H. map-bound declaration is {M.DECLARED} (not a match): "
          f"verdict={row['verdict']} bytematch={row['bytematch']}")
    row = run(lambda n: (None, "symbol not found"))
    check(row["verdict"] == M.MISSING and row["map_reason"] == "symbol not found",
          f"I. missing map symbol stays MISSING: verdict={row['verdict']} "
          f"reason={row['map_reason']!r}")
    row = run(lambda n: (0x180009999, ""))
    check(row["verdict"] == M.MISSING
          and row["map_reason"] == "address 0x180009999 not in inventory",
          f"J. address not in inventory stays MISSING: {row['map_reason']!r}")

    print(f"map self-test: {passed}/{total} passed")
    for n in notes:
        print("  " + n)
    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())