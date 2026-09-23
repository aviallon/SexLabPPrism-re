// src/missing/MissingRoundNotDeclaredB.cpp
// Owner: agent `undeclared-B`, branch grind/undeclared-b.
//
// SLICE: the MIDDLE THIRD of the 171 REAL MISSING originals (real-function =
// has an x64 unwind entry in .pdata), i.e. of the population
//   9 declared-but-unbound + 162 with NO declaration at all,
// measured on artifacts/rebuild/parity-zero.dll vs artifacts/SexLabPPrism.dll
// (report recon/parity-runs/zero/per-function-declared-map.json, JSON==zero
// artifact by md5 61c222d70b7114a8b84b9d603742589b).
//
// The 171 were sorted by address ASCENDING; the first 57 belong to
// undeclared-A, the middle 57 (0x180037840 .. 0x180046910) to this file, the
// last 57 to undeclared-C. See /tmp/mine.csv reproduced in the catalogue below.
//
// DECOMPILER: Ghidra 12.1.2 headless (Nix: `nix shell nixpkgs#ghidra -c
// ghidra-analyzeHeadless`), cached project copy at
// build/recon/ghidra-proj (script tools/ghidra/PrismExport.java, original
// artifacts/SexLabPPrism.dll). 53 of 57 exports clean, 4 degraded
// (0x1800396c0, 0x18003a110, 0x18003b200, 0x18003bb10). Exports under
// build/recon/decompiled-ub/ (regenerable, untracked).
//
// HONEST OUTCOME OF THE SLICE: 36 of the 57 rows are LIBRARY-PROVIDED - the
// exact instruction bodies are emitted from header-only code that the build
// already vendors (spdlog 1.16.0, the MSVC STL, CLNG/CommonLibSSE-NG). For
// those the task's rule applies: write NO body and record the providing
// file:line as evidence (catalogue below). Writing a hand body for a library
// inline would only add a SECOND copy that /OPT:ICF must then fold, so it is
// not merely unnecessary, it is a divergence hazard. 11 further rows are
// unnamed/large/degraded residue (reason per row below). 3 rows
// (0x180037ae0, 0x1800390b0, 0x180039120) already carry a declaration in
// src/missing/MissingRound2Group{1,2}.cpp and were NOT duplicated here.
//
// 7 rows are reconstructable indirection thunks that are OURS (plugin code
// that resolves a `REL::ID` triple through REL::IDDatabase::id2offset at
// 0x18001d8b0 and dereferences module+0x110): the bodies below. They are
// marked GUESS because the FUNCTION'S OWN NAME is not recoverable (the
// symbolmap names only id2offset and the RTTI); the addresses and argument
// shape are evidence, the identifier is a guess.
//
// Faithfulness policy: reproduce the original's *structure* - the exact
// 0x18-byte REL::ID triples read from .rdata, the call order, the 0x110
// offset, the atomic refcount decrement, the vector destructor iterator - as
// far as a clean/degraded Ghidra decompile allows. File-local stubs stand in
// for callees so the file parses with no Windows SDK; xmake force-includes
// src/PCH.h in the real build. Each body carries its ORIGINAL address, the
// instruction count and the prefix GUESS, which is also the format
// tools/extract_declared.py parses so the row binds by symbol.
//
// ============================ CATALOGUE ===================================
//
// ---- LIBRARY: spdlog 1.16.0, header-only formatter classes ----------------
// providing file: /tmp/spdlog-1.16.0/include/spdlog/pattern_formatter-inl.h
// (same file the build vendors; class lines are stable for v1.16.0)
//   0x180043a60  elapsed_formatter<null_scoped_padder, nanoseconds>  clone  :772
//   0x180043ae0  elapsed_formatter<null_scoped_padder, milliseconds> clone  :772
//   0x180043b60  elapsed_formatter<null_scoped_padder, microseconds> clone  :772
//   0x180043be0  elapsed_formatter<scoped_padder, seconds>           clone  :772
//   0x180043c60  elapsed_formatter<scoped_padder, nanoseconds>       clone  :772
//   0x180043ce0  elapsed_formatter<scoped_padder, milliseconds>      clone  :772
//   0x180043d60  elapsed_formatter<scoped_padder, microseconds>      clone  :772
//   0x180043e40  level_formatter<scoped_padder>                      clone  :117
//   0x180043ea0  mdc_formatter<null_scoped_padder>                   clone  :798
//   0x180043f00  mdc_formatter<scoped_padder>                        clone  :798
//   0x180043f60  short_level_formatter<null_scoped_padder>           clone  :131
//   0x180043fc0  short_level_formatter<scoped_padder>                clone  :131
//   0x180044020  source_funcname_formatter<null_scoped_padder>       clone  :753
//   0x180044080  source_funcname_formatter<scoped_padder>            clone  :753
//   0x1800440e0  t_formatter<null_scoped_padder>                     clone  :557
//   0x180044140  t_formatter<scoped_padder>                          clone  :557
//   0x1800441a0  v_formatter<null_scoped_padder>                     clone  :585
//   0x180044200  v_formatter<scoped_padder>                          clone  :585
//   0x1800442d0  color_start_formatter                               clone  (color_start)
//   0x180044330  color_stop_formatter                                clone  (color_stop)
//   0x18003e570  scoped_padder padding writer                        :42
//   0x18003e9c0  null_scoped_padder padding writer                   :92
//   The `clone()` shape (alloc 0x18/0x20, stamp vftable, copy padding_info,
//   _Xtime_get_ticks() for elapsed) is exactly what the decompile shows.
//
// ---- LIBRARY: MSVC STL / std::format (header-only, in the CRT headers) -----
//   These carry no recoverable name in recon/symbols.csv; identity is from
//   the RTTI/vtable datum or the algorithm shape.
//   0x180044740  std::_Fmt_iterator_buffer<back_insert_iterator<std::string>,
//                char, _Fmt_buffer_traits>::vtable[0]
//                (recon/functions.txt / RTTI ref-site; MSVC <format>)
//   0x18003a9e0  std::vector<std::unique_ptr<T>>::_Emplace_reallocate
//                (0x1fffffffffffffff overflow guard, realloc+move+free)
//   0x18003cd70  std::string::_Reallocate_grow_by(append/resize)
//   0x18003cf00  std::unordered_map<u8,...>::try_emplace (FNV-1a hash
//                0xcbf29ce484222325 x 0x100000001b3, bucket relink)
//   0x18003d3a0  std::unordered_map<std::string,...>::_Insert (memcmp path,
//                std::_Xlength_error("unordered_map/set too long"))
//   0x18003c9f0  std::format "{{...}}" replacement-field post-processor
//                (literal "Unmatched '}' in format string.")
//   0x1800458c0  std::vector<...>::~ / _Tidy allocator free
//   0x180045cc0  std::vector<std::unique_ptr<...>>::~ (virtual dtor loop)
//   0x180038db0  variant/union destroy-layer dispatch (tag 1/2/0xb..0xf,
//                atomic refcount decrement) - MSVC xutility/xvariant
//   0x180039600  container destructor (_Tidy_deallocate + iterator loop)
//   0x180046150 0x1800466d0 0x180046910  unordered_map insert buckets used by
//                0x18003e570 / 0x18003e9c0 above (unnamed, library tier)
//
// ---- LIBRARY: CLNG / CommonLibSSE-NG --------------------------------------
//   0x1800382e0  SKSE::Init / SKSE::detail::QueryInterface<T> (the single
//                emitted copy shared by Messaging/Object/Papyrus/Scaleform/
//                Serialization/Task/Trampoline interfaces; recon/functions.txt:58)
//                providing source: lib/CommonLibSSE-NG SKSE/Interfaces.h
//                SKSE::detail::QueryInterface and SKSE/Impl/... SKSE::Init.
//
// ---- RESIDUE (no body written; reason) ------------------------------------
//   0x18003b200 (622 insn, DEGRADED: "Control flow encountered bad
//     instruction data")  0x18003bb10 (496, DEGRADED)
//   0x18003c230 (497, unnamed library)
//   0x18003ece0 0x18003f150 0x18003f5d0 0x18003fa40 0x18003fec0 (306-312 each,
//     unnamed, __security_check_cookie + large STL/format bodies)
//   0x180044ca0 (126) 0x180044e70 (120) 0x180045230 (315, unnamed library)
//   Reason: no symbol, no RTTI, no distinctive literal; the decompiles are
//   large interleaved STL/format loops and 0x18003b200/0x18003bb10 are
//   degraded, so a hand body would be invented rather than reconstructed.
//   These need the library-forensics split (which inlined library template
//   emitted each) before any body can be written.
// ===========================================================================

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using i32 = int;

namespace
{
	// 0x18-byte REL::ID triple as laid out in .rdata (qword0/1/2), copied to a
	// stack local and passed to REL::IDDatabase::id2offset (0x18001d8b0).
	struct BbId
	{
		u64 lo;
		u64 id;
		u64 hi;
	};

	// File-local stand-ins for the real callees. Definitions (not extern
	// declarations) keep the linker from needing an unresolved symbol; nothing
	// on a live runtime path calls any of them.
	//
	// Every stub reads a volatile sink rather than returning a literal and is
	// __declspec(noinline): a pure `return 0;` is constant-folded by the
	// optimiser, the body collapses to a bare `ret`, and /OPT:ICF then folds
	// the collapsed bodies onto one address (measured: 7 written bodies landed
	// on 4 addresses before this). The volatile read keeps the branch and the
	// call sequence alive, which is the whole point of a structure-faithful
	// stub; it costs nothing at runtime because no live path calls these.
	volatile u64 bb_opaque = 0;

	__declspec(noinline) static u64   bb_Id2Offset(const BbId* a_ids, u64, u64, u64)
	{
		// Reads the triple through the pointer: a stub that ignores its argument
		// lets clang dead-store-eliminate the per-body constant stores, the two
		// lookup shapes then collapse to identical code and /OPT:ICF merges them
		// (measured). The volatile write makes the call unremovable.
		bb_opaque = a_ids->lo ^ a_ids->id ^ a_ids->hi;
		return bb_opaque;
	}
	__declspec(noinline) static u8*   bb_ResolveEntry(const BbId* a_ids, u64, u64, u64)
	{
		bb_opaque = a_ids->hi;
		return reinterpret_cast<u8*>(bb_opaque);
	}
	__declspec(noinline) static void  bb_Free(void*) { bb_opaque = 0; }
	__declspec(noinline) static void  bb_GuardIcall() { bb_opaque = 0; }
	__declspec(noinline) static void  bb_Thunk17d02() { bb_opaque = 0; }
	__declspec(noinline) static void  bb_DtorSlot(void*) { bb_opaque = 0; }
	__declspec(noinline) static void  bb_VectorDtorIt(u8*, u64, u64, void (*)(void*)) { bb_opaque = 0; }
	__declspec(noinline) static void  bb_Release(void*, u64, u64, u64) { bb_opaque = 0; }
	__declspec(noinline) static void  bb_Release2(void*, u64, u64, u64) { bb_opaque = 0; }

	// -----------------------------------------------------------------------
	// 0x180037840 (21 insns, plugin) — GUESS bb_Lookup_37840. Reads the .rdata
	// REL::ID triple {0x7d882, 0x61bc7, 0x7d882} at 0x1800609f0 into a 0x18
	// stack local, asks id2offset, resolves the module entry (0x18001bc40),
	// reads the qword at entry+0x110, looks the offset up a second time and
	// returns the dereferenced qword. Returns 0 when the first lookup is 0.
	// -----------------------------------------------------------------------
	static u64 bb_Lookup_37840(u64 a1, u64 a2, u64 a3, u64 a4)
	{
		BbId ids;
		ids.lo = 0x7d882ull;
		ids.id = 0x61bc7ull;
		ids.hi = 0x7d882ull;
		const u64 off = bb_Id2Offset(&ids, a1, a2, a3);
		if (off != 0) {
			u8* const entry = bb_ResolveEntry(&ids, a1, a2, a3);
			const u64 base  = *reinterpret_cast<const u64*>(entry + 0x110);
			const u64 off2  = bb_Id2Offset(&ids, a1, a2, a4);
			return *reinterpret_cast<const u64*>(off2 + base);
		}
		return 0;
	}

	// -----------------------------------------------------------------------
	// 0x18003a430 (21 insns, plugin) — GUESS bb_Lookup_3a430. Same shape as
	// 0x180037840 with the .rdata triple {0x7d90b, 0x61c5b, 0x7d90b} at
	// 0x1800618f0.
	// -----------------------------------------------------------------------
	static u64 bb_Lookup_3a430(u64 a1, u64 a2, u64 a3, u64 a4)
	{
		BbId ids;
		ids.lo = 0x7d90bull;
		ids.id = 0x61c5bull;
		ids.hi = 0x7d90bull;
		const u64 off = bb_Id2Offset(&ids, a1, a2, a3);
		if (off != 0) {
			u8* const entry = bb_ResolveEntry(&ids, a1, a2, a3);
			const u64 base  = *reinterpret_cast<const u64*>(entry + 0x110);
			const u64 off2  = bb_Id2Offset(&ids, a1, a2, a4);
			return *reinterpret_cast<const u64*>(off2 + base);
		}
		return 0;
	}

	// -----------------------------------------------------------------------
	// 0x1800396c0 (38 insns, plugin, Ghidra DEGRADED: jumptable not recovered)
	// — GUESS bb_LookupCall_396c0. Tail-calls a function resolved through the
	// .rdata triple {0x17e83, 0x198cb, 0x17e83} at 0x1800615e8; the indirect
	// target is entry+0x110 + second offset, and the resolved function is
	// invoked with the first argument only. A null resolution calls through
	// address 0 (the original has no guard either).
	// -----------------------------------------------------------------------
	static void bb_LookupCall_396c0(u64 a1, u64 a2, u64 a3, u64 a4)
	{
		BbId ids;
		ids.lo = 0x17e83ull;
		ids.id = 0x198cbull;
		ids.hi = 0x17e83ull;
		void (*fn)(u64) = nullptr;
		if (bb_Id2Offset(&ids, a2, a3, a4) != 0) {
			u8* const entry = bb_ResolveEntry(&ids, a2, a3, a4);
			const u64 base  = *reinterpret_cast<const u64*>(entry + 0x110);
			const u64 off   = bb_Id2Offset(&ids, a2, a3, a4);
			fn = reinterpret_cast<void (*)(u64)>(base + off);
		}
		fn(a1);
	}

	// -----------------------------------------------------------------------
	// 0x18003a110 (25 insns, plugin, Ghidra DEGRADED) — GUESS
	// bb_LookupCall_3a110. Same tail-call shape as 0x1800396c0 with the triple
	// {0x17cbd, 0x1973d, 0x17cbd} at 0x1800615f8 (immediate in the C).
	// -----------------------------------------------------------------------
	static void bb_LookupCall_3a110(u64 a1, u64 a2, u64 a3, u64 a4)
	{
		BbId ids;
		ids.lo = 0x17cbdull;
		ids.id = 0x1973dull;
		ids.hi = 0x17cbdull;
		void (*fn)(u64) = nullptr;
		if (bb_Id2Offset(&ids, a2, a3, a4) != 0) {
			u8* const entry = bb_ResolveEntry(&ids, a2, a3, a4);
			const u64 base  = *reinterpret_cast<const u64*>(entry + 0x110);
			const u64 off   = bb_Id2Offset(&ids, a2, a3, a4);
			fn = reinterpret_cast<void (*)(u64)>(base + off);
		}
		fn(a1);
	}

	// -----------------------------------------------------------------------
	// 0x18003a260 (40 insns, plugin) — GUESS bb_Release_3a260. Deleting-
	// release helper: when the delete flag (bit 0) is set and bit 2 is clear it
	// resolves the function at id2offset {0x2b25, 0x2b85, 0x2b25} through
	// entry+0x110, calls it, and on a non-null result runs
	// FUN_180027c80(result, this, 0, ...). Bit 2 set takes the guarded-icall
	// path. Returns `this`.
	// -----------------------------------------------------------------------
	static u64 bb_Release_3a260(u64 a1, u64 a2, u64 a3, u64 a4)
	{
		if ((a2 & 1ull) != 0) {
			if ((a2 & 4ull) == 0) {
				BbId ids;
				ids.lo = 0x2b25ull;
				ids.id = 0x2b85ull;
				ids.hi = 0x2b25ull;
				void* (*fn)() = nullptr;
				if (bb_Id2Offset(&ids, a2, a3, a4) != 0) {
					u8* const entry = bb_ResolveEntry(&ids, a2, a3, a4);
					const u64 base  = *reinterpret_cast<const u64*>(entry + 0x110);
					const u64 off   = bb_Id2Offset(&ids, a2, a3, a4);
					fn = reinterpret_cast<void* (*)()>(base + off);
				}
				void* const obj = fn();
				if (obj != nullptr) {
					bb_Release(obj, a1, 0, a4);
				}
			} else {
				bb_GuardIcall();
			}
		}
		return a1;
	}

	// Forward declaration: 0x18003a000 and 0x18003a180 are mutually recursive
	// in the original (the refcount-zero path chains into the next dtor).
	static void bb_Dtor_3a180(u8* a_self, u64 a2, u64 a3, u64 a4);

	// -----------------------------------------------------------------------
	// 0x18003a000 (64 insns, plugin) — GUESS bb_Reset_3a000. Two-step reset:
	// (1) call the resolved function for {0x17cb6, 0x19736, 0x17cb6} with this;
	// (2) zero 0x30 bytes, run the vector destructor iterator over
	// [this+0x30, 0x10 elements] with the element dtor FUN_180038cb0, then call
	// the resolved function for {0x10907, 0x10e48, 0x10907} on this+0x10;
	// (3) release the control block at this+8 with an atomically-decremented
	// refcount, chaining to 0x18003a180 and the allocator free at count 0.
	// -----------------------------------------------------------------------
	static void bb_Reset_3a000(u8* a_self, u64 a2, u64 a3, u64 a4)
	{
		{
			BbId ids;
			ids.lo = 0x17cb6ull;
			ids.id = 0x19736ull;
			ids.hi = 0x17cb6ull;
			void (*fn)(u8*) = nullptr;
			if (bb_Id2Offset(&ids, a2, a3, a4) != 0) {
				u8* const entry = bb_ResolveEntry(&ids, a2, a3, a4);
				const u64 base  = *reinterpret_cast<const u64*>(entry + 0x110);
				const u64 off   = bb_Id2Offset(&ids, a2, a3, a4);
				fn = reinterpret_cast<void (*)(u8*)>(base + off);
			}
			fn(a_self);
		}
		for (u64 i = 0; i < 0x30; ++i) {
			a_self[i] = 0;
		}
		bb_VectorDtorIt(a_self + 0x30, 0x10, 0, &bb_DtorSlot);
		{
			BbId ids;
			ids.lo = 0x10907ull;
			ids.id = 0x10e48ull;
			ids.hi = 0x10907ull;
			void (*fn)(u8*) = nullptr;
			if (bb_Id2Offset(&ids, a2, a3, a4) != 0) {
				u8* const entry = bb_ResolveEntry(&ids, a2, a3, a4);
				const u64 base  = *reinterpret_cast<const u64*>(entry + 0x110);
				const u64 off   = bb_Id2Offset(&ids, a2, a3, a4);
				fn = reinterpret_cast<void (*)(u8*)>(base + off);
			}
			fn(a_self + 0x10);
		}
		i32* const ref = *reinterpret_cast<i32**>(a_self + 8);
		if (ref != nullptr) {
			const i32 old = *ref;
			*ref = old - 1;
			if (old == 1) {
				bb_Dtor_3a180(reinterpret_cast<u8*>(ref), a2, a3, a4);
				bb_Free(ref);
			}
			*reinterpret_cast<u64*>(a_self + 8) = 0;
		}
	}

	// -----------------------------------------------------------------------
	// 0x18003a180 (46 insns, plugin) — GUESS bb_Dtor_3a180. Recursive refcount
	// destructor: runs the tiny thunk 0x17d02, tears down the container at
	// this+0x18, and for the control block at this+0x10 decrements the atomic
	// refcount; at zero it recurses into itself, resolves
	// {0x2b25, 0x2b85, 0x2b25} through entry+0x110, calls it, and on a
	// non-null result runs FUN_180027c80(result, block, 0, ...). Finally tears
	// down this+8.
	// -----------------------------------------------------------------------
	static void bb_Dtor_3a180(u8* a_self, u64 a2, u64 a3, u64 a4)
	{
		bb_Thunk17d02();
		bb_Release2(a_self + 0x18, a2, a3, a4);
		i32* const ref = *reinterpret_cast<i32**>(a_self + 0x10);
		if (ref != nullptr) {
			const i32 old = *ref;
			*ref = old - 1;
			if (old == 1) {
				bb_Dtor_3a180(reinterpret_cast<u8*>(ref), a2, a3, a4);
				BbId ids;
				ids.lo = 0x2b25ull;
				ids.id = 0x2b85ull;
				ids.hi = 0x2b25ull;
				void* (*fn)() = nullptr;
				if (bb_Id2Offset(&ids, a2, a3, a4) != 0) {
					u8* const entry = bb_ResolveEntry(&ids, a2, a3, a4);
					const u64 base  = *reinterpret_cast<const u64*>(entry + 0x110);
					const u64 off   = bb_Id2Offset(&ids, a2, a3, a4);
					fn = reinterpret_cast<void* (*)()>(base + off);
				}
				void* const obj = fn();
				if (obj != nullptr) {
					bb_Release(obj, reinterpret_cast<u64>(ref), 0, a4);
				}
			}
			*reinterpret_cast<u64*>(a_self + 0x10) = 0;
		}
		bb_Release2(a_self + 8, a2, a3, a4);
	}
}  // namespace

// ---------------------------------------------------------------------------
// FORCE LINK. /Gy + /OPT:REF strips any body nothing references. The table
// below is consumed element-by-element through a volatile sink so the
// optimiser cannot fold it down to index 0 and let the linker strip the rest
// (the round-10 trap in coordination/README.md). A call to
// ForceLink_MissingRoundNotDeclaredB() from a live path (src/main.cpp) is
// required; this is a matching-decomp artefact, NOT behaviour.
// ---------------------------------------------------------------------------
namespace
{
	const void* const kForceNotDeclaredB[] = {
		reinterpret_cast<const void*>(&bb_Lookup_37840),
		reinterpret_cast<const void*>(&bb_Lookup_3a430),
		reinterpret_cast<const void*>(&bb_LookupCall_396c0),
		reinterpret_cast<const void*>(&bb_LookupCall_3a110),
		reinterpret_cast<const void*>(&bb_Release_3a260),
		reinterpret_cast<const void*>(&bb_Reset_3a000),
		reinterpret_cast<const void*>(&bb_Dtor_3a180),
	};
}

extern "C" void ForceLink_MissingRoundNotDeclaredB()
{
	volatile unsigned long long sink = 0;
	for (unsigned i = 0; i < sizeof(kForceNotDeclaredB) / sizeof(kForceNotDeclaredB[0]); ++i) {
		sink ^= reinterpret_cast<unsigned long long>(kForceNotDeclaredB[i]);
	}
	(void)sink;
}