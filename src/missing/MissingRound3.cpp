#include <cstring>  // memcpy (MSVC has no __builtin_memcpy; the clang syntax check accepted it)
// src/missing/MissingRound3.cpp — round-3 residue reconstructions.
//
// OWNER: missing7 (grind/missing7). This file covers the MISSING real
// (unwind-entry) functions from recon/matching/per-function.json that had NO
// declaration at all in recon/declared-mappings.csv. Computed with:
//   MISSING real (pdata) functions in per-function.json  = 36  (2508 insns)
//   minus every orig_addr in recon/declared-mappings.csv = 25  (2155 insns)
//   => 11 residue addresses (353 insns) -> build/recon/missing-undeclared.csv
//
// Of those 11:
//   * 4 had no body anywhere in src/ and are rebuilt below from Ghidra C
//     (build/recon/decompiled-m7/0x<addr>_target.c, Ghidra 12.1.2, cached
//     project recon/ghidra-proj, script tools/ghidra/PrismExport.java, all 5
//     exports clean — the fifth, 0x180037fe0, is CLNG-provided, see below).
//   * 7 already ship elsewhere and are declared PRESENT-BUT-UNPAIRED so
//     tools/extract_declared.py can bind them; they are NOT duplicated here.
//
// PRESENT-BUT-UNPAIRED (body already in src/; cited so extract_declared binds
// the original address; no second copy is written in this file):
//   0x180008a30  mg2_CopyElems72       src/missing/MissingGroup2.cpp:152
//   0x180016d20  CloseConsolePulse     src/FocusRecovery.cpp:76
//   0x18002c9d0  CopyCatalogAppendFunction  src/missing/MissingRound2Group1.cpp:186
//   0x1800390b0  IdLookup_615a0        src/missing/MissingRound2Group1.cpp:231
//   0x18003a370  mg2_IdJump            src/missing/MissingGroup2.cpp:303
//   0x18003e380  mg2_CopyToBackInserter src/missing/MissingGroup2.cpp:75
//   0x180037fe0  SKSE::MessagingInterface::RegisterListener  src/Lifecycle.cpp:64
//
// The last one is provided inline by the CLNG (`commonlibsse-ng`) dependency
// that xmake links into the DLL; src/Lifecycle.cpp:64 is where our binary
// instantiates it (messaging->RegisterListener("SKSE", OnMessage)). Defining a
// second body would be an ODR clash with CLNG; it is declared, not rebuilt.
//
// Faithfulness policy: the bodies reproduce the original's structure — call
// order, literals, container layout and inlining boundaries — as far as the
// clean Ghidra decompile allows. Each comment carries its ORIGINAL address,
// instruction count and the prefix "GUESS" (the name is a guess from
// behaviour). The file deliberately has NO #include and uses only built-in
// types and raw offsets: it parses under
// `clang --driver-mode=cl -fsyntax-only` with no Windows SDK/sysroot, and
// cannot collide with PCH-provided declarations. xmake force-includes
// src/PCH.h in the real build, so std headers are present there regardless.
//
// FORCE LINK: the parity build is /Gy + /OPT:REF, so an unreferenced body is
// stripped and can never pair. ForceLink_Missing7() below consumes every
// element of the table; src/main.cpp must call it once. This is a
// matching-decomp force-link, NOT behaviour (see the header).

using u8  = unsigned char;
using u32 = unsigned int;
using u64 = unsigned long long;
using i32 = int;
using f32 = float;

namespace
{
	// -----------------------------------------------------------------------
	// File-local stubs for internal callees. Force-link only; nothing on a live
	// runtime path calls any of these. A definition (not an extern declaration)
	// keeps the linker from ever needing an unresolved symbol.
	// -----------------------------------------------------------------------
	static void* mg7_Alloc(u64) { return nullptr; }
	static void  mg7_StrPushBack(struct Mg7String*, char) {}
	static void  mg7_StringAppendRange(struct Mg7String*, const void*, u64) {}
	static void  mg7_VectorAssign(void**, u64, void*) {}
	static int   mg7_MtxLock(void*) { return 0; }
	static void  mg7_MtxUnlock(void*) {}
	static void  mg7_Throw(int) {}
	static void* mg7_GlobalState() { return nullptr; }

	// MSVC std::string layout: +0x00 is the 16-byte union (inline buffer or heap
	// pointer), +0x10 size, +0x18 capacity.
	struct Mg7String
	{
		u8  storage[0x10];
		u64 size;
		u64 cap;
	};

	static u8* mg7_StrBuf(Mg7String* a_str)
	{
		if (a_str->cap > 0xf) {
			return *reinterpret_cast<u8**>(a_str->storage);
		}
		return a_str->storage;
	}

	// -----------------------------------------------------------------------
	// 0x1800257d0 (8 insns, plugin) — GUESS mg7_StringAppendCharThunk. The
	// original loads `this` into rbx, calls the MSVC std::string::push_back at
	// 0x18001daa0 (this in rcx, char in dl) and returns the same string; i.e.
	// `std::string& std::string::operator+=(char)`. The push_back callee is a
	// file-local stub here.
	// -----------------------------------------------------------------------
	static Mg7String* mg7_StringAppendCharThunk(Mg7String* a_str, char a_char)
	{
		mg7_StrPushBack(a_str, a_char);
		return a_str;
	}

	// -----------------------------------------------------------------------
	// 0x18003d600 (library, 70 insns) — GUESS mg7_RehomeStringViews. The
	// original copies a 0x60-byte POD source element (two string_view-like
	// {ptr,len} pairs at +0x00 and +0x50, 0x40 bytes unused between them),
	// default-constructs an inline std::string at +0x60, appends BOTH ranges
	// through 0x18003a630 (StringAppendRange) and then re-points first.ptr at
	// the owner buffer and second.ptr just past first.len. Element size 0x80.
	// -----------------------------------------------------------------------
	struct Mg7ViewElem
	{
		const void* first_ptr;    // 0x00
		u64         first_len;    // 0x08
		u8          reserved[0x40];  // 0x10
		const void* second_ptr;   // 0x50
		u64         second_len;   // 0x58
		Mg7String   owner;        // 0x60
	};

	static Mg7ViewElem* mg7_RehomeStringViews(Mg7ViewElem* a_src, u64 a_count, Mg7ViewElem* a_dst)
	{
		while (a_count != 0) {
			memcpy(a_dst, a_src, 0x60);
			// std::string default: inline buffer, size 0, capacity 0xf.
			*reinterpret_cast<u64*>(a_dst->owner.storage) = 0;
			a_dst->owner.size = 0;
			a_dst->owner.cap  = 0xf;
			a_dst->owner.storage[0] = 0;
			mg7_StringAppendRange(&a_dst->owner, a_dst->first_ptr, a_dst->first_len);
			mg7_StringAppendRange(&a_dst->owner, a_dst->second_ptr, a_dst->second_len);
			// Rehome both views into the concatenated owner buffer.
			const u64 len1 = a_dst->first_len;
			u8*       buf  = mg7_StrBuf(&a_dst->owner);
			a_dst->first_ptr  = buf;
			a_dst->first_len  = len1;
			a_dst->second_ptr = buf + len1;
			a_dst = a_dst + 1;
			a_src = a_src + 1;
			a_count = a_count - 1;
		}
		return a_dst;
	}

	// -----------------------------------------------------------------------
	// 0x180044b80 (plugin, 30 insns) — GUESS mg7_InitRateState. Initializes a
	// 0x40-byte state object: float at +0x00 (last set to 1.0f, 0x3f800000), a
	// std::list sentinel at +0x08 (0x20-byte node self-linked, size +0x10 = 0),
	// an empty std::vector<void*> at +0x18, bounds 7 at +0x30 and 8 at +0x38,
	// then 0x180046db0(vector, 0x10, sentinel) — a 16-element assign.
	// -----------------------------------------------------------------------
	struct Mg7RateState
	{
		f32   rate;        // 0x00
		void* list_head;   // 0x08
		u64   list_size;   // 0x10
		void* vec_begin;   // 0x18
		void* vec_end;     // 0x20
		void* vec_cap;     // 0x28
		u64   lo;          // 0x30
		u64   hi;          // 0x38
	};

	static Mg7RateState* mg7_InitRateState(Mg7RateState* a_self)
	{
		a_self->rate      = 0.0f;
		a_self->list_head = nullptr;
		a_self->list_size = 0;
		u8* node = reinterpret_cast<u8*>(mg7_Alloc(0x20));
		*reinterpret_cast<void**>(node)     = node;
		*reinterpret_cast<void**>(node + 8) = node;
		a_self->list_head = node;
		a_self->vec_begin = nullptr;
		a_self->vec_end   = nullptr;
		a_self->vec_cap   = nullptr;
		a_self->lo        = 7;
		a_self->hi        = 8;
		a_self->rate      = 1.0f;
		mg7_VectorAssign(&a_self->vec_begin, 0x10, a_self->list_head);
		return a_self;
	}

	// -----------------------------------------------------------------------
	// 0x1800483b0 (plugin, 35 insns) — GUESS mg7_SetSuspendFlag. Takes the
	// global state from 0x18004d0f0, locks its mutex (0x18004fb4b), throws
	// std::_Throw_Cpp_error(5) when the lock fails; when state+0x4c is not
	// 0x7fffffff it walks the linked list at state+0xf8 and stores the value
	// into node->[+0x30]->+0x44 (an xchg, i.e. an atomic store), writes
	// state+0x17c = value and unlocks (0x18004fb51). The recursive-lock path
	// sets state+0x4c = 0x7ffffffe first, then throws std::_Throw_Cpp_error(6).
	// -----------------------------------------------------------------------
	struct Mg7SinkNode
	{
		Mg7SinkNode* next;     // 0x00
		Mg7SinkNode* prev;     // 0x08
		u8           pad[0x20];  // 0x10
		void*        payload;  // 0x30
	};

	static void mg7_SetSuspendFlag(i32 a_value)
	{
		u8* state = reinterpret_cast<u8*>(mg7_GlobalState());
		if (mg7_MtxLock(state) != 0) {
			mg7_Throw(5);
			return;
		}
		if (*reinterpret_cast<u32*>(state + 0x4c) != 0x7fffffff) {
			u8* head = *reinterpret_cast<u8**>(state + 0xf8);
			for (u8* n = *reinterpret_cast<u8**>(head); n != head; n = *reinterpret_cast<u8**>(n)) {
				u8* payload = *reinterpret_cast<u8**>(n + 0x30);
				*reinterpret_cast<u32*>(payload + 0x44) = static_cast<u32>(a_value);
			}
			*reinterpret_cast<u32*>(state + 0x17c) = static_cast<u32>(a_value);
			mg7_MtxUnlock(state);
			return;
		}
		*reinterpret_cast<u32*>(state + 0x4c) = 0x7ffffffe;
		mg7_Throw(6);
	}
}

// ---------------------------------------------------------------------------
// FORCE LINK. /Gy + /OPT:REF strips any body nothing references. The table
// below consumes EVERY element (indexed loop into a volatile sink) so the
// optimiser cannot fold the table down to index 0 and let the linker strip the
// rest (the round-10 trap documented in coordination/README.md). A call to
// ForceLink_Missing7() from a live path (src/main.cpp) is required.
// This is a matching-decomp artefact, NOT behaviour.
// ---------------------------------------------------------------------------
namespace
{
	const void* const kForce7[] = {
		reinterpret_cast<const void*>(&mg7_StringAppendCharThunk),
		reinterpret_cast<const void*>(&mg7_RehomeStringViews),
		reinterpret_cast<const void*>(&mg7_InitRateState),
		reinterpret_cast<const void*>(&mg7_SetSuspendFlag),
	};
}

extern "C" void ForceLink_Missing7()
{
	volatile unsigned long long sink = 0;
	for (unsigned i = 0; i < sizeof(kForce7) / sizeof(kForce7[0]); ++i) {
		sink ^= reinterpret_cast<unsigned long long>(kForce7[i]);
	}
	(void)sink;
}
