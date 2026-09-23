// src/missing/MissingRoundNotDeclaredC.cpp — agent `undeclared-C`, branch
// grind/undeclared-c.
//
// SCOPE. This is the LAST THIRD (by ascending original address) of the 162
// MISSING real (.pdata/unwind-entry) functions that had NO declaration at all:
// addresses 0x1800484d0 .. 0x180053f60, 54 functions, 5145 instructions.
// Inputs: artifacts/rebuild/parity-zero.dll vs artifacts/SexLabPPrism.dll,
// report build/recon/matching/per-function.json, declaration set
// build/recon/declared-mappings.csv.
//
// RESULT: every one of the 54 rows is LIBRARY-PROVIDED. No structure-faithful
// plugin body exists to write, so this file deliberately defines NO function
// body and adds NO bindable declaration (a fabricated body here would ODR-clash
// with the dependency and could not pair anyway). The machine-readable evidence
// is recon/undeclared-c-evidence.csv; the decompiler output used for the
// classification is build/recon/decompiled-uc/ (Ghidra 12.1.2, cached project,
// tools/ghidra/PrismExport.java, 54/54 clean, 0 not-found).
//
// WHY "LIBRARY-PROVIDED": 45 of the 54 are the slot-1 `format()` override of a
// spdlog pattern-formatter class, proven by walking the original's RTTI vtables
// in .rdata (COL signature==1 -> type descriptor name). The remaining 9 are
// three spdlog details free functions and six 13-insn MSVC STL <format>
// cleanup funclets. Provider (pinned) is spdlog v1.16.0 and MSVC STL 14.44
// (microsoft/STL tag vs-2022-17.14).
//
// IMPORTANT — these are present in OUR rebuild, they just did not pair. The
// same RTTI vtables exist in artifacts/rebuild/parity-zero.dll: e.g.
//   original  full_formatter::format          0x18004c400
//   rebuild   full_formatter::format          0x1800649e0
//   original  A_formatter<scoped_padder>::format  0x1800484d0
//   rebuild   A_formatter<scoped_padder>::format  0x18005fe90
// (45/54 have a rebuild counterpart; see the CSV column rebuild_format_addr.)
// So the correct remedy for this slice is symbol/address BINDING on the
// dependency's own mangled symbols, not a hand-written body. Writing a body
// would duplicate a symbol the spdlog library already emits.
//
// EVIDENCE TABLE
// (address | provider | providing source file:line | rebuild counterpart)
//   0x1800484d0 | spdlog::details::A_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:172 | rebuild 0x18005fe90
//   0x180048610 | spdlog::details::B_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:207 | rebuild 0x180060050
//   0x180048740 | spdlog::details::C_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:250 | rebuild 0x180060200
//   0x180048830 | spdlog::details::D_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:264 | rebuild 0x180060370
//   0x180048910 | spdlog::details::D_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:264 | rebuild 0x1800603e0
//   0x180048ad0 | spdlog::details::E_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:427 | rebuild 0x1800605b0
//   0x180048c10 | spdlog::details::F_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:412 | rebuild 0x180060770
//   0x180048d30 | spdlog::details::H_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:325 | rebuild 0x180060910
//   0x180048e30 | spdlog::details::I_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:339 | rebuild 0x180060a90
//   0x180048f20 | spdlog::details::M_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:353 | rebuild 0x180060c10
//   0x180049000 | spdlog::details::R_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:478 | rebuild 0x180060d70
//   0x180049080 | spdlog::details::R_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:478 | rebuild 0x180060db0
//   0x1800491c0 | spdlog::details::S_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:367 | rebuild 0x180060f30
//   0x1800492a0 | spdlog::details::T_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:495 | rebuild 0x180061090
//   0x180049370 | spdlog::details::T_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:495 | rebuild 0x1800610e0
//   0x180049510 | spdlog::details::Y_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:283 | rebuild 0x180061290
//   0x180049630 | spdlog::details::a_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:155 | rebuild 0x180061430
//   0x180049770 | spdlog::details::b_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:189 | rebuild 0x1800615f0
//   0x180049870 | spdlog::details::c_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:221 | rebuild 0x180061770
//   0x180049aa0 | spdlog::details::c_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:221 | rebuild 0x180061870
//   0x180049db0 | spdlog::details::d_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:311 | rebuild 0x180061aa0
//   0x180049ef0 | spdlog::details::e_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:381 | rebuild 0x180061c60
//   0x18004a170 | spdlog::details::elapsed_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:772 | rebuild 0x180061f70
//   0x18004a2f0 | spdlog::details::elapsed_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:772 | rebuild 0x180062170
//   0x18004a5d0 | spdlog::details::elapsed_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:772 | rebuild 0x180062540
//   0x18004a7b0 | spdlog::details::f_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:396 | rebuild 0x1800627a0
//   0x18004a900 | spdlog::details::level_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:117 | rebuild 0x180062970
//   0x18004aa00 | spdlog::details::m_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:297 | rebuild 0x180062af0
//   0x18004ab20 | spdlog::details::mdc_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:798 | rebuild 0x180062c90
//   0x18004abf0 | spdlog::details::name_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:104 | rebuild 0x180062df0
//   0x18004ad20 | spdlog::details::p_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:443 | rebuild 0x180062fa0
//   0x18004ae40 | spdlog::details::pid_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:571 | rebuild 0x180063140
//   0x18004af60 | spdlog::details::r_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:457 | rebuild 0x1800632f0
//   0x18004b0a0 | spdlog::details::r_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:457 | rebuild 0x180063390
//   0x18004b2e0 | spdlog::details::short_filename_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:693 | rebuild 0x1800635b0
//   0x18004b500 | spdlog::details::short_level_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:131 | rebuild 0x180063840
//   0x18004b640 | spdlog::details::source_filename_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:675 | rebuild 0x180063a00
//   0x18004b7f0 | spdlog::details::source_funcname_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:753 | rebuild 0x180063c60
//   0x18004b970 | spdlog::details::source_linenum_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:734 | rebuild 0x180063e90
//   0x18004bb00 | spdlog::details::source_location_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:646 | rebuild 0x1800640d0
//   0x18004bdd0 | spdlog::details::t_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:557 | rebuild 0x1800643d0
//   0x18004bf10 | spdlog::details::v_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:585 | rebuild 0x1800645a0
//   0x18004bff0 | spdlog::details::z_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:514 | rebuild 0x180064700
//   0x18004c150 | spdlog::details::z_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:514 | rebuild 0x1800647b0
//   0x18004c400 | spdlog::details::full_formatter::format | extern/spdlog/include/spdlog/pattern_formatter-inl.h:839 | rebuild 0x1800649e0
//   0x18004d240 | spdlog::details::fmt_helper::pad2 | extern/spdlog/include/spdlog/details/fmt_helper.h:84
//   0x18004de30 | spdlog::details::os::utc_minutes_offset | extern/spdlog/include/spdlog/details/os-inl.h:247
//   0x18004e1c0 | spdlog::details::file_helper::open | extern/spdlog/include/spdlog/details/file_helper-inl.h:26 (throw :60)
//   0x180052e80 | MSVC STL <format>/<chrono> SEH cleanup funclet | microsoft/STL vs-2022-17.14 stl/inc/format (buffer cleanup); no direct caller
//   0x180053010 | MSVC STL <format>/<chrono> SEH cleanup funclet | microsoft/STL vs-2022-17.14 stl/inc/format (buffer cleanup); no direct caller
//   0x1800532d0 | MSVC STL <format>/<chrono> SEH cleanup funclet | microsoft/STL vs-2022-17.14 stl/inc/format (buffer cleanup); no direct caller
//   0x1800534e0 | MSVC STL <format>/<chrono> SEH cleanup funclet | microsoft/STL vs-2022-17.14 stl/inc/format (buffer cleanup); no direct caller
//   0x180053b00 | MSVC STL <format>/<chrono> SEH cleanup funclet | microsoft/STL vs-2022-17.14 stl/inc/format (buffer cleanup); no direct caller
//   0x180053f60 | MSVC STL <format>/<chrono> SEH cleanup funclet | microsoft/STL vs-2022-17.14 stl/inc/format (buffer cleanup); no direct caller
//
// NO FORCE-LINK is present because there is no body to keep: /Gy + /OPT:REF
// only strips defined functions, and this file defines none.
