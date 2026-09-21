// MissingRound2Group2.cpp — relaunch round (agent `missing5`), group g2.
//
// Bodies for the 8 g2 addresses in build/recon/missing-full.csv that had NO
// counterpart anywhere in src/ at the time of writing. The other 12 g2 rows
// already have address-labelled bodies in src/missing/MissingGroup{1,2,3}.cpp
// and src/FocusRecovery.cpp; they are reported PRESENT-BUT-UNPAIRED, not
// duplicated here.
//
// Each function carries its ORIGINAL address and the reason for its guessed
// name. Decompiles: build/recon/decompiled-g5/0x<addr>_anon.c (Ghidra 12.1.2,
// cached project build/recon/ghidra-proj, script tools/ghidra/PrismExport.java).
//
// Faithfulness policy: the body reproduces the original's structure — call
// order, literals and containers — as far as the decompilation allows. The
// bodies are force-link-only and never executed, so internal callees are
// file-local stubs that only have to satisfy the call graph.
//
// The file deliberately has NO #include and uses only built-in types and raw
// offsets: it then parses under `clang --driver-mode=cl -fsyntax-only` with no
// Windows SDK/sysroot, and cannot collide with PCH-provided declarations.
// xmake force-includes src/PCH.h in the real build, so std headers are present
// there regardless.

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using i32 = int;
using i64 = long long;

namespace
{
	// -----------------------------------------------------------------------
	// File-local stubs for internal callees. Force-link only; nothing on a
	// live path calls any of these. A definition (not an extern declaration)
	// keeps the linker from ever needing an unresolved symbol.
	// -----------------------------------------------------------------------
	static void* mg5_Alloc(u64) { return nullptr; }
	static void  mg5_Free(void*) {}
	static void  mg5_AtExit(void*) {}
	static void  mg5_Throw(int) {}
	static void  mg5_Terminate() {}
	static int   mg5_ThrdId() { return 0; }
	static int   mg5_ThrdJoin(void*, int) { return 0; }
	static void  mg5_ThreadJoin(void*) {}
	static void  mg5_MtxLock(void*) {}
	static void  mg5_MtxUnlock(void*) {}
	static void  mg5_Destroy(void*, int) {}
	static u32   mg5_AtomicSub(volatile u32* p, u32 n) { const u32 prev = *p; *p = prev - n; return prev; }
	static void  mg5_StrGrow(struct Mg5String*, u64, char) {}
	static char  mg5_CheckGuard(void*) { return 0; }
	static u64   mg5_Id2Offset(const u64*, u64, u64, u64) { return 0; }
	static void* mg5_ResolveId(const u64*, u64, u64, u64) { return nullptr; }
	static void  mg5_FormatArgs(u64*, const void*, const u64*, void*) {}
	static void  mg5_Format(u64*, const u64*, const void*, int, void*) {}
	static void  mg5_Publish(void*, const void*, bool, char) {}
	static void  mg5_FmtUInt(u32, struct Mg5String*) {}

	// MSVC std::back_insert-style sink: field0 points at a function-pointer
	// slot; the grow call is `(**(code**)sink)(sink)`.
	struct Mg5Sink
	{
		void** vptr;  // +0x00
		u8*    data;  // +0x08
		u64    size;  // +0x10
		u64    cap;   // +0x18
	};

	static void mg5_SinkAppend(Mg5Sink* a_sink, u8 a_byte)
	{
		if (a_sink->cap < a_sink->size + 1) {
			void (*grow)(Mg5Sink*) = *reinterpret_cast<void (**)(Mg5Sink*)>(a_sink->vptr);
			grow(a_sink);
		}
		a_sink->data[a_sink->size] = a_byte;
		a_sink->size += 1;
	}

	// MSVC std::string layout: +0x00 is the 16-byte union (inline buffer or
	// heap pointer), +0x10 size, +0x18 capacity.
	struct Mg5String
	{
		u8  storage[0x10];
		u64 size;
		u64 cap;
	};

	static u8* mg5_StrBuf(Mg5String* a_str)
	{
		if (a_str->cap > 0xf) {
			return *reinterpret_cast<u8**>(a_str->storage);
		}
		return a_str->storage;
	}

	// Equivalent of 0x180007dd0(str, 1, ?, c) when the string is full.
	static void mg5_StrAppend(Mg5String* a_str, char a_char)
	{
		const u64 size = a_str->size;
		if (size < a_str->cap) {
			u8* base         = mg5_StrBuf(a_str);
			base[size]       = static_cast<u8>(a_char);
			base[size + 1]   = 0;
			a_str->size      = size + 1;
		} else {
			mg5_StrGrow(a_str, 1, a_char);
		}
	}

	// -----------------------------------------------------------------------
	// 0x180001100 (12 insns, library) — dynamic initializer for the global
	// std::string at 0x18009c190: allocates a 0x20-byte buffer, copies the
	// literal `{"active":false}` (16 bytes) plus NUL, stores the sibling
	// globals from .rdata 0x18005fd50 ({0x10, 0x1f} = {size, capacity}) and
	// registers the atexit destructor at 0x180054ef0.
	// GUESS mg5_InitActiveFalseGlobal.
	// -----------------------------------------------------------------------
	struct Mg5GlobalString
	{
		u64 ptr;
		u64 size;
		u64 cap;
	};

	static Mg5GlobalString g5_activeFalse;

	static void mg5_InitActiveFalseGlobal()
	{
		static const u8 kLiteral[16] = { '{', '"', 'a', 'c', 't', 'i', 'v', 'e', '"', ':', 'f', 'a', 'l', 's', 'e', '}' };
		u8*             buf          = reinterpret_cast<u8*>(mg5_Alloc(0x20));
		g5_activeFalse.size          = 0x10;
		g5_activeFalse.cap           = 0x1f;
		g5_activeFalse.ptr           = reinterpret_cast<u64>(buf);
		for (u64 i = 0; i < 16; ++i) {
			buf[i] = kLiteral[i];
		}
		buf[0x10] = 0;
		mg5_AtExit(reinterpret_cast<void*>(&mg5_InitActiveFalseGlobal));
	}

	// -----------------------------------------------------------------------
	// 0x180010b20 (55 insns, library) — `std::thread::~thread()`: join when
	// joinable (throwing `system_error` 1/5/2 on detached/self-join/join
	// failure), release the shared state with the two atomic decrements, and
	// `std::terminate()` if the thread is still joinable.
	// GUESS mg5_ThreadDtor.
	// -----------------------------------------------------------------------
	static void mg5_ThreadDtor(u64* a_thread)
	{
		if (a_thread[1] != 0) {
			mg5_ThreadJoin(a_thread + 2);
			if (a_thread[1] == 0) {
				mg5_Throw(1);
			}
			if (static_cast<u32>(a_thread[1]) == static_cast<u32>(mg5_ThrdId())) {
				mg5_Throw(5);
			}
			u32 local[4];
			const u64 id = a_thread[1];
			local[0]     = static_cast<u32>(a_thread[0]);
			local[1]     = static_cast<u32>(a_thread[0] >> 32);
			local[2]     = static_cast<u32>(id);
			local[3]     = static_cast<u32>(id >> 32);
			if (mg5_ThrdJoin(local, 0) != 0) {
				mg5_Throw(2);
			}
			a_thread[0] = 0;
			a_thread[1] = 0;
		}
		i32* shared = reinterpret_cast<i32*>(a_thread[2]);
		if (shared != nullptr) {
			const u32 prev = mg5_AtomicSub(reinterpret_cast<volatile u32*>(shared + 1), 2);
			if ((prev & 0xfffffffeu) == 2u) {
				if (mg5_AtomicSub(reinterpret_cast<volatile u32*>(shared), 1) == 1) {
					mg5_Free(shared);
				}
			}
		}
		if (a_thread[1] != 0) {
			mg5_Terminate();
		}
	}

	// -----------------------------------------------------------------------
	// 0x180044510 (88 insns, library) — decimal writer for 0..999: three
	// single-digit appends into an MSVC std::string, falling back to the
	// general unsigned formatter at 0x1800405d0 above 999.
	// GUESS mg5_AppendTripleDigit.
	// -----------------------------------------------------------------------
	static void mg5_AppendTripleDigit(u32 a_value, Mg5String* a_out)
	{
		if (a_value > 999) {
			mg5_FmtUInt(a_value, a_out);
			return;
		}
		const char hundreds = static_cast<char>(a_value / 100) + '0';
		mg5_StrAppend(a_out, hundreds);
		const char tens = static_cast<char>((a_value % 100) / 10);
		mg5_StrAppend(a_out, static_cast<char>(tens + '0'));
		mg5_StrAppend(a_out, static_cast<char>((a_value % 100) + tens * -10 + '0'));
	}

	// -----------------------------------------------------------------------
	// 0x18003e380 (44 insns, library) — copy [first,last) into a
	// back_insert_iterator, growing through the sink's function slot.
	// (A same-named body already lives in MissingGroup2.cpp; this local copy
	// exists only so the g5 call graph below links.)
	// -----------------------------------------------------------------------
	static Mg5Sink* mg5_CopyToSink(Mg5Sink* a_out, const u8* a_first, const u8* a_last, Mg5Sink* a_sink)
	{
		for (; a_first != a_last; ++a_first) {
			mg5_SinkAppend(a_sink, *a_first);
		}
		return a_out;
	}

	// -----------------------------------------------------------------------
	// 0x180046050 (76 insns, library) — integer formatter core: emits the
	// two-character prefix from the spec, optional sign flag, then the
	// zero-padding count `digits - minWidth`, then copies the digit range into
	// the sink. Returns the output iterator.
	// GUESS mg5_WritePrefixedInt.
	// -----------------------------------------------------------------------
	struct Mg5FmtInt
	{
		const u8*       prefix;      // +0x00 (two bytes)
		const u8*       signFlag;    // +0x08 (points at a char)
		const i32*      minWidth;    // +0x10
		const i32*      digits;      // +0x18
		const u8*       valueFirst;  // +0x20
		const u8* const* valueLast;  // +0x28
	};

	static Mg5Sink* mg5_WritePrefixedInt(Mg5Sink* a_out, const Mg5FmtInt* a_spec, Mg5Sink* a_sink)
	{
		const u8* q = a_spec->prefix;
		for (const u8* end = q + 2; q != end; ++q) {
			mg5_SinkAppend(a_sink, *q);
		}
		if (*a_spec->signFlag != 0) {
			if (*a_spec->minWidth < *a_spec->digits) {
				for (i32 n = *a_spec->digits - *a_spec->minWidth; n > 0; --n) {
					mg5_SinkAppend(a_sink, 0x30);
				}
			}
		}
		mg5_CopyToSink(a_out, a_spec->valueFirst, *a_spec->valueLast, a_sink);
		return a_out;
	}

	// -----------------------------------------------------------------------
	// 0x18003e400 (115 insns, library) — the `<format>` aligned write: reads
	// the format spec (`width` at +0x00, `align` at +0x09, fill length at
	// +0x0e, fill bytes at +0x0f), decides the before/after fill counts for
	// left (1)/right (2)/center (3) alignment, emits the fill runs around the
	// value written by 0x180046050.
	// GUESS mg5_WriteAlignedFormat.
	// -----------------------------------------------------------------------
	static Mg5Sink* mg5_WriteAlignedFormat(Mg5Sink* a_out, const u8* a_spec, i32 a_count, i32* a_specInt, char a_defaultFill, void* a_value)
	{
		const i32  width = *reinterpret_cast<const i32*>(a_spec);
		char       align = reinterpret_cast<const char*>(a_spec)[9];
		if (align == 0) {
			align = a_defaultFill;
		}
		i32 beforeCount = 0;
		i32 afterCount  = 0;
		if (a_count < width) {
			if (align == 1) {
				afterCount = width - a_count;
			} else if (align == 2) {
				beforeCount = width - a_count;
			} else if (align == 3) {
				beforeCount = (width - a_count) / 2;
				afterCount  = (width - beforeCount) - a_count;
			}
		}
		const u64  fillLen = reinterpret_cast<const u8*>(a_spec)[0xe];
		const u8*  fill    = reinterpret_cast<const u8*>(a_spec) + 0xf;
		for (i32 n = 0; n < beforeCount; ++n) {
			for (u64 i = 0; i < fillLen; ++i) {
				mg5_SinkAppend(a_out, fill[i]);
			}
		}
		Mg5Sink* after = mg5_WritePrefixedInt(a_out, static_cast<const Mg5FmtInt*>(a_value), a_out);
		for (i32 n = 0; n < afterCount; ++n) {
			for (u64 i = 0; i < fillLen; ++i) {
				mg5_SinkAppend(after, fill[i]);
			}
		}
		(void)a_specInt;
		return after;
	}

	// -----------------------------------------------------------------------
	// 0x180022e60 (112 insns, plugin) — guarded publication of a UI state
	// frame: checks the guard at param_1+0x88, builds the format argument
	// store from param_4/param_5, links param_1's inline string
	// (size at +0x20, capacity at +0x18) plus param_2's triple, runs the
	// formatter 0x180044c00 and publishes through 0x18004d1f0. Frees the
	// temporary string when it spilled to the heap.
	// GUESS mg5_PublishActiveState.
	// -----------------------------------------------------------------------
	struct Mg5StringView
	{
		u64 buf;
		u64 size;
		u64 cap;
	};

	static void mg5_PublishActiveState(i64* a_state, i64* a_frame, i32 a_index, i64* a_spec, u64* a_value)
	{
		const i64 count = a_state[8];
		const char guard = mg5_CheckGuard(a_state + 0x11);
		if (static_cast<i32>(count) <= a_index || guard != 0) {
			Mg5String temporary;
			temporary.size = 0;
			temporary.cap  = 0xf;
			u64 argPair[2] = { static_cast<u64>(*a_spec), static_cast<u64>(a_spec[1]) };
			void* argTable[2] = { reinterpret_cast<void*>(1), argPair };
			u64  argStore[2]  = { 0, 0 };
			mg5_FormatArgs(argStore, &temporary, argPair, argTable);

			Mg5StringView view;
			view.buf  = reinterpret_cast<u64>(a_state + 1);
			if (static_cast<u64>(a_state[4]) > 0xf) {
				view.buf = static_cast<u64>(a_state[1]);
			}
			view.size = 0;
			view.cap  = static_cast<u64>(a_state[3]);

			u64 frame[3] = { static_cast<u64>(*a_frame), static_cast<u64>(a_frame[1]), static_cast<u64>(a_frame[2]) };
			u64 formatted[24] = {};
			mg5_Format(formatted, frame, &view, a_index, argTable);
			mg5_Publish(a_state, formatted, static_cast<i32>(count) <= a_index, guard);
			if (temporary.cap > 0xf) {
				mg5_Free(mg5_StrBuf(&temporary));
			}
		}
		(void)a_value;
	}

	// -----------------------------------------------------------------------
	// 0x18004d830 (102 insns, plugin) — drains the pending-work container
	// under the mutex: guards the recursion counter at +0x4c, swaps the
	// smart-pointer slot at +0x170, walks the intrusive list at +0xf8 and for
	// every node runs the per-entry virtual dispatch over the node's
	// [0x28,0x30) pair range, then releases the old slot pointer.
	// GUESS mg5_DispatchPendingTasks.
	// -----------------------------------------------------------------------
	static void mg5_DispatchPendingTasks(i64 a_owner, i64* a_slot, u64 a_arg3, u64 a_arg4)
	{
		(void)a_arg3;
		(void)a_arg4;
		mg5_MtxLock(reinterpret_cast<void*>(a_owner));
		i32* recursion = reinterpret_cast<i32*>(a_owner + 0x4c);
		if (*recursion == 0x7fffffff) {
			*recursion = 0x7ffffffe;
			mg5_Throw(6);
		}
		u64  next = static_cast<u64>(*a_slot);
		*a_slot   = 0;
		void* previous = *reinterpret_cast<void**>(a_owner + 0x170);
		*reinterpret_cast<u64*>(a_owner + 0x170) = next;
		if (previous != nullptr) {
			mg5_Destroy(previous, 1);
		}
		u8* sentinel = *reinterpret_cast<u8**>(a_owner + 0xf8);
		for (u8* node = *reinterpret_cast<u8**>(sentinel); node != sentinel; node = *reinterpret_cast<u8**>(node)) {
			u8* payload = *reinterpret_cast<u8**>(node + 0x30);
			u8* first   = *reinterpret_cast<u8**>(payload + 0x28);
			u8* last    = *reinterpret_cast<u8**>(payload + 0x30);
			for (u8* it = first; it != last; it += 0x10) {
				u8* entry = *reinterpret_cast<u8**>(it);
				void (*apply)(void*, void*) = *reinterpret_cast<void (**)(void*, void*)>(*reinterpret_cast<u64*>(entry) + 0x20);
				void* slot = reinterpret_cast<void*>(it);
				if (it + 0x10 == last) {
					u64 local = reinterpret_cast<u64>(slot);
					apply(entry, &local);
					break;
				}
				apply(entry, reinterpret_cast<void*>(slot));
			}
		}
		mg5_MtxUnlock(reinterpret_cast<void*>(a_owner));
		if (u64 remaining = static_cast<u64>(*a_slot)) {
			mg5_Destroy(reinterpret_cast<void*>(remaining), 1);
		}
	}

	// -----------------------------------------------------------------------
	// 0x180037ae0 (21 insns, plugin) — SKSE `REL::IDDatabase` address
	// resolver: seeds the 3-qword ID structure from .rdata 0x180060e58
	// ({0x7d8ed, 0x61c3d, 0x7d8ed}), asks for the offset, resolves the
	// relocation base (0x18001bc40) and returns the pointer stored at
	// base+0x110 + second-offset.
	// GUESS mg5_RelIdLookup.
	// -----------------------------------------------------------------------
	static u64 mg5_RelIdLookup(u64 a_id, u64 a_off, u64 a_arg3, u64 a_arg4)
	{
		static const u64 kSeed[3] = { 0x7d8edu, 0x61c3du, 0x7d8edu };
		u64              ids[3]   = { kSeed[0], kSeed[1], kSeed[2] };
		u64              offset   = mg5_Id2Offset(ids, a_id, a_off, a_arg3);
		if (offset != 0) {
			u8* resolved = reinterpret_cast<u8*>(mg5_ResolveId(ids, a_id, a_off, a_arg3));
			u64 base     = *reinterpret_cast<u64*>(resolved + 0x110);
			u64 finalOff = mg5_Id2Offset(&ids[0], a_id, a_off, a_arg4);
			return *reinterpret_cast<u64*>(finalOff + base);
		}
		return 0;
	}
}  // namespace

// ---------------------------------------------------------------------------
// Force-link table. The parity build uses /Gy + /OPT:REF, so a body nothing
// references is stripped and can never be paired. Taking the address of every
// reconstructed function in a translation-unit array keeps each body alive.
// The indexed loop with a volatile sink consumes every element; reading only
// kForce5[0] would let the optimiser constant-fold the rest away and the
// linker strip the bodies (the /OPT:REF trap, README round 10).
// This is the ONLY reason the table exists.
// ---------------------------------------------------------------------------
namespace
{
	const void* const kForce5[] = {
		reinterpret_cast<const void*>(&mg5_InitActiveFalseGlobal),
		reinterpret_cast<const void*>(&mg5_ThreadDtor),
		reinterpret_cast<const void*>(&mg5_AppendTripleDigit),
		reinterpret_cast<const void*>(&mg5_WritePrefixedInt),
		reinterpret_cast<const void*>(&mg5_WriteAlignedFormat),
		reinterpret_cast<const void*>(&mg5_PublishActiveState),
		reinterpret_cast<const void*>(&mg5_DispatchPendingTasks),
		reinterpret_cast<const void*>(&mg5_RelIdLookup),
	};
}

extern "C" void ForceLink_Missing5Group2()
{
	volatile unsigned long long sink = 0;
	for (unsigned i = 0; i < sizeof(kForce5) / sizeof(kForce5[0]); ++i) {
		sink ^= reinterpret_cast<unsigned long long>(kForce5[i]);
	}
	(void)sink;
}
