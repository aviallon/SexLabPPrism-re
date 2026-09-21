
## 2026-09-20 — missing2 (grind/missing2)
- 32 g2 rows; 21 bodies emitted + force-linked in src/missing/MissingGroup2.{h,cpp}
- 2 (0x1800261e0 ApplyPresentation, 0x1800255c0 outer RequestSearchInput) already partly in bridge src (Presentation.cpp / ActionDispatch.cpp)
- 11 library/STL internals documented, not duplicated (ICF/inlined)
- main.cpp hook needed: ForceLink_MissingGroup2();
- Ghidra exports: build/recon/decompiled-g2/
