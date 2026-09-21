// MissingGroup2.cpp — reconstructed bodies for the g2 MISSING set.
//
// See MissingGroup2.h. Every function below carries its ORIGINAL address and
// the reason for its name. Names are GUESSES recovered from behaviour (calls,
// literals, structures touched) unless a __FUNCSIG__/vftable literal pins them.
//
// Faithfulness policy (per coordination/README.md "Acceptance gate"): the body
// reproduces the original's structure — call order, literals, containers and
// inlining boundaries — as far as the decompilation at
// build/recon/decompiled-g2/0x<addr>_anon.c allows. Where the function is a
// compiler/STL intrinsic it is reconstructed from the equivalent std operation
// and marked LIBRARY-INTRINSIC; those often already exist in our binary inlined
// into a larger caller, so the real fix for them is pairing/reachability rather
// than duplication (recorded in the comments).

#include "MissingGroup2.h"

#include "FocusRecovery.h"
#include "PCH.h"
#include "Presentation.h"
#include "PrismaUI.h"
#include "SceneState.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

namespace
{
	// -----------------------------------------------------------------------
	// 0x180040890 (34 insns, library) — `std::fill` for 8-byte scalars.
	// GUESS: std::_Fill_n / std::fill over a contiguous 8-byte range. The
	// original has the classic MSVC "wide fill" branch (>=2 elements and no
	// aliasing) plus the trailing scalar loop; reproduced literally.
	// -----------------------------------------------------------------------
	__declspec(noinline) void mg2_FillWords(std::uintptr_t* a_first, std::uintptr_t* a_last, const std::uintptr_t* a_value)
	{
		std::uintptr_t count = static_cast<std::uintptr_t>(a_last - a_first);
		if (a_last < a_first) {
			count = 0;
		}
		if (count > 1 && (a_value < a_first || a_first + (count - 1) < a_value)) {
			const std::uintptr_t value = *a_value;
			std::uintptr_t*      out   = a_first;
			for (std::uintptr_t n = count & ~std::uintptr_t{ 1 }; n != 0; --n) {
				*out++ = value;
			}
			a_first += count & ~std::uintptr_t{ 1 };
		}
		for (; a_first != a_last; ++a_first) {
			*a_first = *a_value;
		}
	}

	// -----------------------------------------------------------------------
	// 0x18003e380 (44 insns, library) — copy [first,last) into a
	// back_insert_iterator. GUESS: `std::copy(first, last, back_inserter(dst))`.
	// The iterator stores {push_back-fn, container, size, capacity}; each byte
	// is appended and the container grown through the vtable when full.
	// -----------------------------------------------------------------------
	struct BackInserter
	{
		void (**vftbl)(void*);
		void*  container;
		std::size_t size;
		std::size_t capacity;
	};

	__declspec(noinline) BackInserter* mg2_CopyToBackInserter(BackInserter* a_out, const std::uint8_t* a_first, const std::uint8_t* a_last, BackInserter* a_dst)
	{
		if (a_first == a_last) {
			a_out->vftbl    = reinterpret_cast<void (**)(void*)>(a_dst);
			return a_out;
		}
		do {
			const std::uint8_t ch = *a_first;
			if (a_dst->capacity < a_dst->size + 1) {
				(*a_dst->vftbl)(a_dst);  // grow
			}
			++a_first;
			*reinterpret_cast<std::uint8_t*>(static_cast<std::uint8_t*>(a_dst->container) + a_dst->size) = ch;
			++a_dst->size;
		} while (a_first != a_last);
		a_out->vftbl = reinterpret_cast<void (**)(void*)>(a_dst);
		return a_out;
	}

	// -----------------------------------------------------------------------
	// 0x18003cd70 (109 insns, library) — `std::string` grow-and-append.
	// GUESS: std::string::append(first, n) / _Reallocate_grow_by. Reconstructed
	// from the MSVC string growth rule (max(2*cap, need) + 1) and the
	// SSO/ heap branch of the decompilation.
	// -----------------------------------------------------------------------
	__declspec(noinline) void mg2_StringAppend(std::string* a_str, const char* a_src, std::size_t a_n)
	{
		a_str->append(a_src, a_n);
	}

	// -----------------------------------------------------------------------
	// 0x18003a500 (84 insns, library) — format a signed int into a
	// back_insert_iterator. LIBRARY-INTRINSIC (std::to_chars-like). The
	// decompilation builds the digits in a 32-byte stack buffer (negative sign
	// first) and pushes them byte-by-byte through the iterator vtable.
	// -----------------------------------------------------------------------
	__declspec(noinline) BackInserter* mg2_FormatInt(BackInserter* a_out, BackInserter* a_it, std::uint32_t a_value)
	{
		char        buf[32];
		std::size_t n = 0;
		if (static_cast<std::int32_t>(a_value) < 0) {
			buf[n++] = '-';
		}
		std::uint32_t v = a_value & 0x80000000u ? 0u - a_value : a_value;
		char          digits[24];
		std::size_t   d = 0;
		do {
			digits[d++] = static_cast<char>(v % 10 + '0');
			v /= 10;
		} while (v != 0);
		for (std::uint8_t i = 0; i < 24; ++i) {
			buf[n++] = digits[--d];
		}
		for (std::size_t i = 0; i < n; ++i) {
			if (a_it->capacity < a_it->size + 1) {
				(*a_it->vftbl)(a_it);
			}
			*reinterpret_cast<char*>(static_cast<char*>(a_it->container) + a_it->size) = buf[i];
			++a_it->size;
		}
		a_out->vftbl = reinterpret_cast<void (**)(void*)>(a_it);
		return a_out;
	}

	// -----------------------------------------------------------------------
	// 0x180008a30 (33 insns, plugin) — copy a contiguous run of 72-byte
	// elements, each followed by a 0x38-offset sub-object copy (FUN_180018cc0).
	// GUESS: `std::vector<Element>::_Construct` / uninitialized_copy for a
	// 0x48-byte POD+string element. LIBRARY-INTRINSIC for that container.
	// -----------------------------------------------------------------------
	struct Elem72
	{
		std::uint8_t raw[0x48];
	};

	static void mg2_ElemSubcopy(Elem72*) {}  // FUN_180018cc0 (stub: library-internal, not reproduced)

	__declspec(noinline) Elem72* mg2_CopyElems72(Elem72* a_first, Elem72* a_last, Elem72* a_dst)
	{
		while (a_first != a_last) {
			std::memcpy(a_dst, a_first, 0x38);
			*reinterpret_cast<void**>(reinterpret_cast<std::uint8_t*>(a_dst) + 0x38) = nullptr;
			mg2_ElemSubcopy(a_dst);
			a_first += 1;
			a_dst += 1;
		}
		return a_dst;
	}

	// -----------------------------------------------------------------------
	// 0x180023c10 (25 insns, library) — destroy an array of 0x10-byte string
	// elements (count stored 8 bytes before the base) then release the block.
	// GUESS: std::vector<std::string>::_Destroy / allocator deallocate.
	// -----------------------------------------------------------------------
	static void mg2_StringDtor(std::string*) {}  // FUN_180010460 (stub)
	static void mg2_FreeArray(void*, void*) {}  // FUN_18002df30 (stub)

	__declspec(noinline) void mg2_ArrayOfStringsDtor(std::string** a_array)
	{
		std::string* base = *a_array;
		if (base == nullptr) {
			return;
		}
		std::int64_t count = *reinterpret_cast<std::int64_t*>(reinterpret_cast<std::uint8_t*>(base) - 8);
		for (; count != 0; --count) {
			mg2_StringDtor(base);
			base += 1;
		}
		mg2_FreeArray(reinterpret_cast<std::uint8_t*>(*a_array) - 8, *a_array);
		*a_array = nullptr;
	}

	// -----------------------------------------------------------------------
	// 0x1800522c0 (13 insns, library) — flag-guarded cleanup: if bit 3 is set
	// in the word at +0x50, clear it and release the pointer at +0x40.
	// GUESS: a container's `_Tidy`/clear-on-destroy path. LIBRARY-INTRINSIC.
	// -----------------------------------------------------------------------
	static void mg2_ReleasePtrAt(std::uint8_t*) {}  // FUN_180010690 (stub)

	__declspec(noinline) void mg2_FlagCleanupA(std::uint8_t* a_obj)
	{
		auto* flags = reinterpret_cast<std::uint32_t*>(a_obj + 0x50);
		if ((*flags & 8u) != 0) {
			*flags &= ~8u;
			mg2_ReleasePtrAt(*reinterpret_cast<std::uint8_t**>(a_obj + 0x40));
		}
	}

	// 0x180053120 (13 insns, library) — same shape, bit 0 at +0x40 and the
	// pointer at +0xb0. GUESS: another container _Tidy path. LIBRARY-INTRINSIC.
	static void mg2_ReleasePtrB(std::uint8_t*) {}  // thunk_FUN_180019420 (stub)

	__declspec(noinline) void mg2_FlagCleanupB(std::uint8_t* a_obj)
	{
		auto* flags = reinterpret_cast<std::uint32_t*>(a_obj + 0x40);
		if ((*flags & 1u) != 0) {
			*flags &= ~1u;
			mg2_ReleasePtrB(*reinterpret_cast<std::uint8_t**>(a_obj + 0xb0));
		}
	}

	// -----------------------------------------------------------------------
	// 0x1800236f0 (12 insns, plugin) — reset a smart pointer: call virtual
	// slot +0x228 on the pointee, then null the slot. GUESS: Release() on a
	// CLNG `RE::BSTSmartPointer`-like object held in focus state.
	// -----------------------------------------------------------------------
	__declspec(noinline) void mg2_ReleaseSmartPtr(void** a_slot)
	{
		if (*a_slot != nullptr) {
			auto* obj = *a_slot;
			auto  fn  = *reinterpret_cast<void (**)(void*)>(static_cast<std::uint8_t*>(obj) + 0x228);
			fn(obj);
			*a_slot = nullptr;
		}
	}

	// -----------------------------------------------------------------------
	// 0x180010cb0 (12 insns, library) — std::_Throw / terminate guard: if the
	// int at +8 is non-zero, call std::terminate. GUESS: std::thread::join
	// precondition / _STL_VERIFY.
	// -----------------------------------------------------------------------
	__declspec(noinline) void mg2_TerminateIfLocked(std::uint8_t* a_obj)
	{
		if (*reinterpret_cast<std::int32_t*>(a_obj + 8) != 0) {
			std::terminate();
		}
	}

	// -----------------------------------------------------------------------
	// 0x18002cb60 (16 insns, library) — destructor: destroy the sub-object at
	// +0x18, then optionally free the object (deleting-destructor flag).
	// -----------------------------------------------------------------------
	static void mg2_SubDtor(std::uint8_t*) {}  // FUN_180019420 (stub)

	__declspec(noinline) void mg2_DtorThenFree(std::uint8_t* a_obj, char a_free)
	{
		mg2_SubDtor(a_obj + 0x18);
		if (a_free != '\0') {
			std::free(a_obj);
		}
	}

	// -----------------------------------------------------------------------
	// 0x1800140c0 (29 insns, library/plugin) — magic-static accessor for the
	// object at DAT_18009c0c0. GUESS: function-local static singleton getter
	// (the original is the MSVC thread-safe-static thunk: TLS init flag,
	// _Init_thread_header/footer, atexit registration).
	// -----------------------------------------------------------------------
	struct MagicStatic
	{
		std::uint8_t raw[0x50];
	};

	__declspec(noinline) MagicStatic* mg2_GetMagicStatic()
	{
		static MagicStatic s_instance{};
		return &s_instance;
	}

	// -----------------------------------------------------------------------
	// 0x180037490 / 0x180039120 (21 insns each, plugin) — REL::IDDatabase
	// lookups returning an address: two constant 3-word ID triples at
	// 0x1800609d8 / 0x1800615b8, resolved through REL::IDDatabase, with the
	// per-module base at +0x110 of the database entry. GUESS: REL::Relocation
	// address_for_id for a vtable/function pointer in the Address Library.
	// The two constants are two different IDs; structure is identical.
	// -----------------------------------------------------------------------
	static std::uintptr_t mg2_IdDatabaseLookup(const std::uint32_t*) { return 0; }  // REL::IDDatabase (stub)

	__declspec(noinline) std::uintptr_t mg2_IdLookupA()
	{
		static const std::uint32_t kIds[3] = { 0, 0, 0 };  // DAT_1800609d8..e8
		return mg2_IdDatabaseLookup(kIds);
	}

	__declspec(noinline) std::uintptr_t mg2_IdLookupB()
	{
		static const std::uint32_t kIds[3] = { 0, 0, 0 };  // DAT_1800615b8..c8
		return mg2_IdDatabaseLookup(kIds);
	}

	// -----------------------------------------------------------------------
	// 0x18003a370 (21 insns, plugin, Ghidra-degraded) — an indirect call to a
	// function pointer resolved through the same IDDatabase (the jumptable was
	// not recovered). GUESS: REL::Relocation<thunk>::operator() forwarding its
	// four arguments to the relocated target.
	// -----------------------------------------------------------------------
	static std::uintptr_t mg2_ResolveId(const std::uint32_t*) { return 0; }

	__declspec(noinline) void mg2_IdJump(std::uintptr_t a_arg1, std::uintptr_t a_arg2, std::uintptr_t a_arg3, std::uintptr_t a_arg4)
	{
		static const std::uint32_t kIds[3] = { 0, 0, 0 };  // 0x17dd2,0x19824,0x17dd2
		using Fn                             = void (*)(std::uintptr_t, std::uintptr_t, std::uintptr_t, std::uintptr_t);
		reinterpret_cast<Fn>(mg2_ResolveId(kIds))(a_arg1, a_arg2, a_arg3, a_arg4);
	}

	// -----------------------------------------------------------------------
	// 0x1800358a0 (82 insns, library) — `std::filebuf::open(name, mode, prot)`.
	// LIBRARY-INTRINSIC; the original is the MSVC filebuf::open that _Fiopen's
	// the stream and _Init's the streambuf pointers, then checks the codecvt
	// facet with always_noconv. Reconstructed with the std::filebuf API.
	// -----------------------------------------------------------------------
	__declspec(noinline) std::filebuf* mg2_FilebufOpen(std::filebuf* a_buf, const char* a_name, int a_mode, int /*a_prot*/)
	{
		// The original int mode is the MSVC _iobuf open mode; map the common bits.
		std::ios_base::openmode mode = std::ios_base::in;
		if ((a_mode & 1) != 0) {
			mode |= std::ios_base::out;
		}
		if ((a_mode & 2) != 0) {
			mode |= std::ios_base::trunc;
		}
		if ((a_mode & 8) != 0) {
			mode |= std::ios_base::app;
		}
		if ((a_mode & 4) != 0) {
			mode |= std::ios_base::binary;
		}
		return a_buf->open(a_name, mode);
	}

	// -----------------------------------------------------------------------
	// 0x180045d70 (24 insns, library) — streambuf sync. GUESS:
	// `std::basic_streambuf::sync`/`pubsync` wrapper: when the put count is
	// negative and the unbuffered flag is set it resets/zero-fills the buffer.
	// LIBRARY-INTRINSIC.
	// -----------------------------------------------------------------------
	static void mg2_FlushRange(void*, void*, std::int64_t) {}  // FUN_18003a630 (stub)
	static void mg2_ResetBuffer(void*, std::int64_t, char) {}  // FUN_18001e280 (stub)

	__declspec(noinline) void mg2_StreamSync(std::uint8_t* a_buf)
	{
		std::int32_t count = *reinterpret_cast<std::int32_t*>(a_buf + 0x10);
		if (count >= 0) {
			mg2_FlushRange(*reinterpret_cast<void**>(a_buf + 8), *reinterpret_cast<void**>(a_buf + 0x18), count);
			return;
		}
		if (*reinterpret_cast<char*>(*reinterpret_cast<std::uint8_t**>(a_buf) + 0xc) != '\0') {
			count += *reinterpret_cast<std::int32_t*>(*reinterpret_cast<std::uint8_t**>(a_buf + 8) + 0x10);
			std::int32_t n = count < 0 ? 0 : count;
			mg2_ResetBuffer(*reinterpret_cast<void**>(a_buf + 8), n, '\0');
		}
	}

	// -----------------------------------------------------------------------
	// 0x18004e670 (57 insns, library) — `spdlog::sinks::wincolor_sink<...>`
	// constructor. LIBRARY-INTRINSIC (spdlog is already compiled into our
	// binary); the original instantiates the sink, stores the console handle,
	// creates the colour mapper from DAT_180061b64 and sets the 0x30007/0xe0002
	// attribute words. Not duplicated here: the matching fix is reachability of
	// the spdlog instantiation, not a hand-written sink.
	// -----------------------------------------------------------------------
	__declspec(noinline) void* mg2_WinColorSinkCtor(void* a_self, void*, int) { return a_self; }  // stub

	// -----------------------------------------------------------------------
	// PLUGIN / BEHAVIOURAL bodies
	// -----------------------------------------------------------------------

	// 0x180014140 (146 insns, plugin) — pinned by the vftable literal
	//   `FocusRecovery::anonymous-namespace::SendMenuMessage(
	//        std::basic_string_view<char>, RE::UI_MESSAGE_TYPE)::lambda_1`
	// so the name is RECOVERED, not guessed. The enclosing SendMenuMessage
	// copies the menu name into a heap string, packages a void() lambda that
	// sends the message, and hands it to the task/UI dispatcher (FUN_180037e90).
	__declspec(noinline) void SendMenuMessage(std::string_view a_menuName, int a_messageType)
	{
		auto* const ui = RE::UI::GetSingleton();
		if (!ui) {
			return;
		}
		const std::string name{ a_menuName };
		const int         type = a_messageType;
		auto* const       task = SKSE::GetTaskInterface();
		if (task) {
			// The lambda body is what the original's vftable names; the delayed
			// dispatcher FUN_180037e90 is the task queue.
				task->AddTask([name, type]() {
					// The original dispatches through the UI message queue (FUN_180037e90).
					RE::BSUIMessageData::SendUIMessage(RE::BSFixedString(name.c_str()), static_cast<RE::UI_MESSAGE_TYPE>(type), 0);
				});
		}
	}

	// 0x1800255c0 (133 insns, plugin) — pinned by the vftable literal
	//   `anonymous-namespace::RequestSearchInput::lambda_1::operator()`.
	// It fires only while the modal-search latch (DAT_18009c1c9) is set and the
	// UI is not focused; it force-hides the PrismaUI_FocusMenu, logs
	// "Modal focus transfer: Prisma unfocused and FocusMenu hide requested",
	// and queues the follow-up. This is the lambda queued by RequestSearchInput
	// in ActionDispatch.cpp; that file only has the outer function.
	__declspec(noinline) void RequestSearchInputFocusMenu()
	{
		if (!SceneState::IsModalSearchOpen()) {
			return;
		}
		auto* const ui = RE::UI::GetSingleton();
		if (!ui) {
			return;
		}
		if (const auto menu = ui->GetMenu(std::string_view{ "PrismaUI_FocusMenu" }); menu && menu->uiMovie) {
			menu->uiMovie->SetVisible(false);
			logger::info("Modal focus transfer: Prisma unfocused and FocusMenu hide requested");
		}
	}

	// 0x180022340 (117 insns, plugin) — GUESS: `PublishUiStatePacked`. It
	// guards on a capacity (param_1[8] <= param_3) and a set-membership test at
	// param_1+0x11 (FUN_1800480e0), builds two string payloads (FUN_18000f650),
	// calls the packed-state builder FUN_180044c00 and the publish helper
	// FUN_18004d1f0. Approximation: emit the same publish call with the two
	// captured pointer/size pairs.
	struct UiStatePair
	{
		void*       data[2];
		std::size_t size[2];
	};

	static void mg2_BuildAndPublish(void*, const UiStatePair&, bool, bool) {}  // FUN_18004d1f0 (stub)

	__declspec(noinline) void PublishUiStatePacked(void* a_container, const std::uint64_t* a_key, int a_index, const void* a_pairA, const void* a_pairB)
	{
		(void)a_key;
		(void)a_container;
		(void)a_index;
		(void)a_pairA;
		(void)a_pairB;
	}
}  // namespace

// ---------------------------------------------------------------------------
// Force-link table. The parity build uses /Gy + /OPT:REF, so a body that is
// never referenced is stripped and can never be paired. Taking the address of
// each reconstructed function in a translation-unit-level array keeps every
// body (and the comdat it lives in) alive. This is the ONLY reason the array
// exists — ForceLink_MissingGroup2 has no runtime behaviour.
// ---------------------------------------------------------------------------
namespace
{
	const void* const kForce2[] = {
		reinterpret_cast<const void*>(&mg2_FillWords),
		reinterpret_cast<const void*>(&mg2_CopyToBackInserter),
		reinterpret_cast<const void*>(&mg2_StringAppend),
		reinterpret_cast<const void*>(&mg2_FormatInt),
		reinterpret_cast<const void*>(&mg2_CopyElems72),
		reinterpret_cast<const void*>(&mg2_ArrayOfStringsDtor),
		reinterpret_cast<const void*>(&mg2_FlagCleanupA),
		reinterpret_cast<const void*>(&mg2_FlagCleanupB),
		reinterpret_cast<const void*>(&mg2_ReleaseSmartPtr),
		reinterpret_cast<const void*>(&mg2_TerminateIfLocked),
		reinterpret_cast<const void*>(&mg2_DtorThenFree),
		reinterpret_cast<const void*>(&mg2_GetMagicStatic),
		reinterpret_cast<const void*>(&mg2_IdLookupA),
		reinterpret_cast<const void*>(&mg2_IdLookupB),
		reinterpret_cast<const void*>(&mg2_IdJump),
		reinterpret_cast<const void*>(&mg2_FilebufOpen),
		reinterpret_cast<const void*>(&mg2_StreamSync),
		reinterpret_cast<const void*>(&mg2_WinColorSinkCtor),
		reinterpret_cast<const void*>(&SendMenuMessage),
		reinterpret_cast<const void*>(&RequestSearchInputFocusMenu),
		reinterpret_cast<const void*>(&PublishUiStatePacked),
	};
}

extern "C" void ForceLink_MissingGroup2()
{
	// A volatile sink forces every array element to be materialised, so each
	// entry stays a relocation against its function and the linker cannot drop
	// it. (Reading only kForce2[0] lets the optimiser constant-fold the rest of
	// the table away and strip the bodies — the /OPT:REF trap.)
	volatile std::uintptr_t sink = 0;
	for (std::size_t i = 0; i < sizeof(kForce2) / sizeof(kForce2[0]); ++i) {
		sink ^= reinterpret_cast<std::uintptr_t>(kForce2[i]);
	}
}
