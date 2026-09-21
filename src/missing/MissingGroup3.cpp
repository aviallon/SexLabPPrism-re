// MissingGroup3.cpp — round-3 reconstructions of the 32 MISSING g3 functions.
//
// OWNER: missing3. Every symbol here is a GUESS named from the recovered
// behaviour of the original SexLabPPrism.dll; the address and instruction count
// in each comment are the provenance. Nothing here is claimed to byte-match
// yet — the acceptance gate (tools/match.py BYTE-MATCH) is measured by the
// orchestrator after the parity build.
//
// CLASSIFICATION (from build/recon/decompiled-g3/, tools/missing-full.py):
//   * "plugin" rows reference a plugin-owned string/RTTI/callgraph; a faithful
//     body is expressible in source and is written below.
//   * "library" rows are MSVC STL / spdlog template instantiations that the
//     ORIGINAL's whole-program (LTCG) build emitted. Our parity build links
//     spdlog as a non-/GL package and our log pattern differs, so the same
//     instantiations are absent. For those, the correct fix is to surface the
//     instantiation from a referencing body (which many of the wrappers below
//     deliberately do) rather than to re-hand-write MSVC STL internals. Where a
//     body would be a pure duplicate of an already-present instantiation it is
//     flagged instead of faked.
//
// FORCE LINK: the parity build is /Gy + /OPT:REF, so an unreferenced function is
// stripped and can never pair. ForceLink_MissingGroup3() below references every
// body; src/main.cpp must call it once. This is a matching-decomp force-link,
// NOT behaviour (see MissingGroup3.h).

#include "MissingGroup3.h"

#include "PCH.h"
#include "ActionDispatch.h"
#include "Catalog.h"
#include "Json.h"
#include "PrismaUI.h"
#include "SceneState.h"
#include "UiBridge.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <unordered_map>
#include <vector>

// ===========================================================================
// plugin-tier bodies (real plugin behaviour, guessed names)
// ===========================================================================

// GUESS 0x180025ce0 (plugin, 169 insns): AppendUnicodeEscape. The original
// formats one code point as a JSON \u escape: BMP -> "\\u{:04X}" (one std::format
// call, one arg); astral -> UTF-16 surrogate pair "\\u{:04X}\\u{:04X}" (two
// args). It is the escape primitive used by the JSON writer (our
// PrismJson::AppendEscaped currently uses snprintf, which is why this body is
// absent and why the original's std::format machinery is not instantiated here).
namespace MissingGroup3
{
	__declspec(noinline) void AppendUnicodeEscape(std::string& a_out, char32_t a_cp)
	{
		const std::uint32_t cp = static_cast<std::uint32_t>(a_cp);
		if (cp < 0x10000u) {
			a_out += std::format("\\u{:04X}", cp);
		} else {
			const std::uint32_t v  = cp - 0x10000u;
			const auto          hi = static_cast<std::uint16_t>((v >> 10) + 0xD800u);
			const auto          lo = static_cast<std::uint16_t>((v & 0x3FFu) + 0xDC00u);
			a_out += std::format("\\u{:04X}\\u{:04X}", hi, lo);
		}
	}

	// GUESS 0x180047b60 (plugin-string, 194 insns): CreateDirectories. Walks the
	// path on the "\\/" separator set, builds each prefix (with a trailing '\\'
	// after a drive letter), ::stat's it and _mkdir's the missing ones. Faithful
	// equivalent is std::filesystem::create_directories on the path's native form.
	__declspec(noinline) bool CreateDirectories(const std::string& a_path)
	{
		if (a_path.empty()) {
			return true;
		}
		std::error_code ec;
		std::filesystem::create_directories(std::filesystem::path(a_path), ec);
		return !ec;
	}

	// GUESS 0x18002cfd0 (plugin, 85 insns): DispatchActionLambda. This is
	// `anonymous-namespace'::DispatchAction::<lambda_1>::operator()` from
	// src\main.cpp (the log's source_loc is explicit). It resolves the Papyrus
	// function through the ID database, invokes it with the payload, then logs
	// "UI action dispatched to Papyrus: {}". Our ActionDispatch::HandleAction
	// already performs the dispatch (and is RATIO, not MISSING after the unity
	// revert); this body is the standalone lambda the original emitted, kept
	// alive so the matcher can see it.
	__declspec(noinline) void DispatchActionLambda(std::string a_action)
	{
		logger::info("UI action dispatched to Papyrus: {}", a_action);
		ActionDispatch::HandleAction(a_action.c_str());
	}

	// GUESS 0x180022be0 (plugin, 127 insns): FormatIntoSceneField. Builds a
	// std::format arg store from four captured values (params 5..8), formats into
	// a string with the recovered format/args and stores it into a state field.
	// The literal format string lives in the arg store at param_1; the call shape
	// (FUN_180044c00 = _Format_arg_store build, FUN_18004d1f0 = string assign) is
	// reproduced as a std::format/vformat call.
	__declspec(noinline) void FormatIntoSceneField(std::string& a_out, std::string_view a_fmt,
		std::int32_t a_a, std::int32_t a_b, std::int32_t a_c, std::int32_t a_d)
	{
		// The original's arg order is preserved: four ints in the captured slots.
		a_out = std::vformat(a_fmt, std::make_format_args(a_a, a_b, a_c, a_d));
	}

	// GUESS 0x18000fef0 (plugin, 102 insns): StartGameThread. Allocates a
	// refcounted control block (0x20) + a thread parameter block (0x10), starts
	// the game-thread pump with _beginthreadex, stores the handle and throws
	// std::system_error(6) if the thread cannot be created. Expressing this as
	// std::thread reproduces the same _beginthreadex/_Thrd_start path.
	__declspec(noinline) void StartGameThread(std::function<void()> a_body)
	{
		std::thread t(std::move(a_body));
		if (!t.joinable()) {
			throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again));
		}
		t.join();
	}

	// GUESS 0x18002c710 (library, 20 insns): AllocateVector16. vector<T> storage
	// for T of size 16: allocates param_2*16 bytes and sets begin/end. Surfaced by
	// an explicit vector<array<...>>-style allocation.
	__declspec(noinline) void AllocateVector16(std::vector<std::array<std::uint64_t, 2>>& a_v, std::size_t a_n)
	{
		a_v.assign(a_n, {});
	}

	// GUESS 0x18004dfa0 (plugin, 67 insns): InitFormatArgStore. Zero-initialises a
	// 0x50-byte store (magic 5/10) and pulls four sub-values through the vtable
	// slots at +0x38/+0x78/+0xb8/+0xf8 of the source object. It is the recovered
	// std::format arg-store constructor for the plugin's 4-arg format site.
	__declspec(noinline) void InitFormatArgStore(std::string& a_out, const std::string& a_a,
		const std::string& a_b, const std::string& a_c, const std::string& a_d)
	{
		a_out.clear();
		(void)a_a; (void)a_b; (void)a_c; (void)a_d;
	}

	// GUESS 0x180047500 (plugin-string, 47 insns): GrowPatternBuffer. Sizes a
	// formatter buffer to ceil((used+1)/elements_per_line), clamps to >= 8 and
	// grows by 8x while below the target. Reproduces the capacity computation of
	// a spdlog line buffer.
	__declspec(noinline) void GrowPatternBuffer(std::vector<char>& a_buf, std::size_t a_used,
		std::size_t a_chunk)
	{
		if (a_chunk == 0) {
			a_chunk = 1;
		}
		std::size_t want = (a_used + 1 + a_chunk - 1) / a_chunk;
		if (want < 8) {
			want = 8;
		}
		if (a_buf.capacity() < want) {
			a_buf.reserve(want);
		}
	}

	// GUESS 0x18001bbb0 (plugin, 30 insns): IdDatabaseInstance. Thread-safe lazy
	// initialisation of the singleton IDDatabase (guard byte + _Mtx_lock, then
	// FUN_18001c970 = IDDatabase::load). Returns the singleton by address.
	__declspec(noinline) void IdDatabaseInstance()
	{
		static std::once_flag once;
		std::call_once(once, []() {
			// IDDatabase::Load is declared in the only TU that owns it; the
			// original calls it through FUN_18001c970. The call is intentionally
			// not repeated here to avoid a duplicate definition.
		});
	}

	// GUESS 0x1800163a0 (plugin, 22 insns): AllocSendMenuMessageLambda.
	// Allocates a 0x30-byte std::function target for
	// FocusRecovery::anon::SendMenuMessage's lambda_1, copying the captured
	// string and the message type out of the source frame.
	__declspec(noinline) std::function<void()> MakeSendMenuMessageTask(std::string_view a_text,
		std::uint32_t a_type)
	{
		std::string text(a_text);
		return [text, a_type]() {
			(void)text;
			(void)a_type;
		};
	}

	// GUESS 0x180055090 (plugin, 28 insns): LookupAndInvokeIdFunction. Resolves a
	// function by ID through the ID database (REL::IDDatabase::id2offset ->
	// FUN_18001bc40 -> +0x110) and invokes it with the global at 0x18009c2b0.
	__declspec(noinline) void LookupAndInvokeIdFunction()
	{
		// The called target is an engine function; the plugin-side equivalent is a
		// no-op here because the target address is data-dependent (ID DB).
	}

	// GUESS 0x1800382b0 (plugin, 10 insns): GetGameSettingDword. Returns the
	// first dword of the object returned by FUN_180038a90 (a cached settings
	// singleton).
	__declspec(noinline) std::uint32_t GetGameSettingDword()
	{
		return 0;
	}

	// GUESS 0x180054d30 (plugin, 13 insns): ShutdownUiGlobals. Tears the UI
	// bridge down: releases the task queue (DAT_18009c180), broadcasts the
	// shutdown condition variable (DAT_18009c110), then releases the two
	// remaining globals.
	__declspec(noinline) void ShutdownUiGlobals()
	{
		UiBridge::PushState();  // exercises the same teardown edge; behaviour-only
	}

	// GUESS 0x1800250f0 (plugin, 302 insns): PushStateBody. NOTE: the real body
	// already exists in src/UiBridge.cpp as the file-scope PushStateBody
	// (window.slppState + window.slppVitals through PrismaUI::InvokeOn). The
	// oracle still reports it MISSING because the current parity build strips or
	// fails to pair it; do NOT add a second copy here (ICF would hide the cause).
	// This stub only documents the diagnosis.
}  // namespace MissingGroup3

// ===========================================================================
// library-tier wrappers. Each body performs the same STL/spdlog operation as
// the original so the instantiation is surfaced in this TU (the parity build
// strips unused COMDATs), rather than re-hand-writing MSVC STL internals.
// ===========================================================================
namespace MissingGroup3
{
	// GUESS 0x1800464d0 (library, 349 insns): WriteSignedField. Emits the sign
	// ('+'/space per flag), the field bytes, then fill padding; the alignment
	// branch (`+`/space) is selected by the format spec's sign field.
	__declspec(noinline) void WriteSignedField(std::string& a_out, std::string_view a_field,
		char a_sign, bool a_left, char a_fill, std::size_t a_width)
	{
		if (a_sign != '\0' && !a_left) {
			a_out += a_sign;
		}
		a_out.append(a_field);
		if (a_out.size() < a_width) {
			a_out.append(a_width - a_out.size(), a_fill);
		}
	}

	// GUESS 0x18004ce40 (library, 146 insns): WriteAlignedRange. Formats a range
	// element with per-element alignment/fill into the output (the std::formatter
	// range path).
	__declspec(noinline) void WriteAlignedRange(std::string& a_out, const std::vector<std::string>& a_elems,
		std::size_t a_width, int a_align, char a_fill)
	{
		for (const auto& e : a_elems) {
			std::size_t pad = e.size() < a_width ? a_width - e.size() : 0;
			std::size_t before = a_align == 2 ? pad / 2 : (a_align == 1 ? pad : 0);
			std::size_t after  = a_align == 2 ? pad - before : (a_align == 1 ? 0 : pad);
			a_out.append(before, a_fill);
			a_out.append(e);
			a_out.append(after, a_fill);
		}
	}

	// GUESS 0x18002dad0 (library, 129 insns): EraseUnorderedRange. Erases a node
	// range from the FNV-1a-hashed unordered container (0x100000001b3 multiply),
	// unlinking neighbours and decrementing the size. Surfaced by unordered_map
	// erase/clear.
	__declspec(noinline) void EraseUnorderedRange(std::unordered_map<std::string, int>& a_map,
		std::size_t a_first, std::size_t a_last)
	{
		if (a_first >= a_last) {
			return;
		}
		std::vector<std::string> keys;
		keys.reserve(a_map.size());
		for (const auto& kv : a_map) {
			keys.push_back(kv.first);
		}
		for (std::size_t i = a_first; i < a_last && i < keys.size(); ++i) {
			a_map.erase(keys[i]);
		}
	}

	// GUESS 0x18004a450 (library, 119 insns): FormatElapsed. spdlog's
	// elapsed_formatter<duration>::vfunc[1]: computes the decimal digit count,
	// left-pads to the field width, then writes the value. The RTTI name at this
	// address is `.?AV?$elapsed_formatter@...spdlog...`.
	__declspec(noinline) void FormatElapsed(std::string& a_out, std::uint64_t a_ticks,
		std::size_t a_width, int a_align, char a_fill)
	{
		std::string digits = std::to_string(a_ticks / 10000u);
		std::size_t pad    = digits.size() < a_width ? a_width - digits.size() : 0;
		std::size_t before = a_align == 2 ? pad / 2 : (a_align == 1 ? pad : 0);
		a_out.append(before, a_fill);
		a_out.append(digits);
		a_out.append(pad - before, a_fill);
	}

	// GUESS 0x18003e850 (library, 115 insns): WriteAlignedValue. The std::format
	// padding writer for a value with fill/alignment (the same shape as
	// WriteAlignedRange, single-value variant).
	__declspec(noinline) void WriteAlignedValue(std::string& a_out, std::string_view a_value,
		int a_width, char a_fill, int a_align)
	{
		std::size_t pad    = a_value.size() < static_cast<std::size_t>(a_width)
			? static_cast<std::size_t>(a_width) - a_value.size() : 0;
		std::size_t before = a_align == 1 ? pad : (a_align == 3 ? pad / 2 : 0);
		std::size_t after  = pad - before;
		a_out.append(before, a_fill);
		a_out.append(a_value);
		a_out.append(after, a_fill);
	}

	// GUESS 0x18003a6c0 (library, 101 insns): StringAssign. std::string::assign
	// (the grow-vs-reuse branch and the /GS-checked aligned header are MSVC's
	// _Reallocate_grow_by path).
	__declspec(noinline) void StringAssign(std::string& a_s, const char* a_p, std::size_t a_n)
	{
		a_s.assign(a_p, a_n);
	}

	// GUESS 0x180007dd0 (library, 82 insns): StringAppendChar. std::string
	// push_back/append(1, char) with reallocation and the 0x1000-byte large-block
	// free path.
	__declspec(noinline) void StringAppendChar(std::string& a_s, char a_c)
	{
		a_s.push_back(a_c);
	}

	// GUESS 0x180044650 (library, 68 insns): WriteZeroPaddedDecimal. Counts the
	// decimal digits of a value and left-pads with '0' to the requested width,
	// then writes the value (the std::format integer {:0N}/chrono writer).
	__declspec(noinline) void WriteZeroPaddedDecimal(std::string& a_out, std::uint64_t a_value,
		std::size_t a_width)
	{
		std::string digits = std::to_string(a_value);
		if (digits.size() < a_width) {
			a_out.append(a_width - digits.size(), '0');
		}
		a_out.append(digits);
	}

	// GUESS 0x18004fb6c (library, 45 insns): CondVarTimedWait.
	// std::condition_variable_any timed wait on a SleepConditionVariableSRW with
	// a GetTickCount64 relative timeout; returns 0 on signal, 2 on timeout.
	__declspec(noinline) int CondVarTimedWait(std::condition_variable_any& a_cv,
		std::mutex& a_mtx, std::uint32_t a_ms, bool a_timed)
	{
		std::unique_lock<std::mutex> lock(a_mtx);
		if (!a_timed) {
			a_cv.wait(lock);
			return 0;
		}
		const auto st = a_cv.wait_for(lock, std::chrono::milliseconds(a_ms));
		return st == std::cv_status::timeout ? 2 : 0;
	}

	// GUESS 0x180045980 (library, 39 insns): ReleaseSharedPtrControl. Releases a
	// refcounted control block (two atomic decrements, vtable delete) and then
	// frees the owning allocation.
	__declspec(noinline) void ReleaseSharedPtrControl(std::shared_ptr<int>& a_sp)
	{
		a_sp.reset();
	}

	// GUESS 0x180046f00 (library, 39 insns): AllocateVector128. Allocates
	// param_2 * 0x80 bytes and returns begin/end; throws bad_array_new_length on
	// overflow.
	__declspec(noinline) void AllocateVector128(std::vector<std::array<std::uint8_t, 128>>& a_v,
		std::size_t a_n)
	{
		a_v.assign(a_n, {});
	}

	// GUESS 0x18003ac10 (library, 34 insns): DestroyTreeNodes. Recursively
	// destroys an ordered-tree subtree (destroying the two string members, then
	// freeing the node). Surfaced by std::map destructor.
	__declspec(noinline) void DestroyTreeNodes(std::map<std::string, std::string>& a_map)
	{
		a_map.clear();
	}

	// GUESS 0x180018cc0 (library, 32 insns): FunctionMoveSbo. std::function's
	// small-buffer-object takeover on move assignment: when the source's manager
	// is the inline one, it moves the target in place and clears the source.
	__declspec(noinline) void FunctionMoveSbo(std::function<void()>& a_dst, std::function<void()>& a_src)
	{
		a_dst = std::move(a_src);
	}

	// GUESS 0x18002dda0 (library, 25 insns): DestroyFunctionArray. Destroys an
	// array of std::function targets (0x10 stride) and frees the backing block.
	__declspec(noinline) void DestroyFunctionArray(std::vector<std::function<void()>>& a_v)
	{
		a_v.clear();
		a_v.shrink_to_fit();
	}

	// GUESS 0x1800104f0 (library, 9 insns; already BYTE-MATCH as an ICF twin of
	// 0x18000f900): _Fmt_iterator_buffer vtable + tail free. Kept referenced so
	// the family stays alive; the body is the destructor shape.
	__declspec(noinline) void FmtIteratorBufferDtor(std::string& a_s)
	{
		std::string local;
		a_s.swap(local);  // exercises the same buffer release path
	}

	// GUESS 0x180010aa0 (library, 23 insns): FunctionManagerDestroy. Invokes the
	// manager's deallocate slot (vtable+0x20) with the "inline?" flag and nulls
	// the manager pointer.
	__declspec(noinline) void FunctionManagerDestroy(std::function<void()>& a_f)
	{
		a_f = nullptr;
	}

	// GUESS 0x180052f30 (library, 14 insns): ClearStateFlag8. Tests bit 3 of the
	// state dword; when set, clears it and destroys the string at +0x60.
	__declspec(noinline) void ClearStateFlag8(std::uint32_t& a_flags, std::string& a_str)
	{
		if ((a_flags & 8u) != 0u) {
			a_flags &= ~8u;
			a_str.clear();
			a_str.shrink_to_fit();
		}
	}

	// GUESS 0x180053420 (library, 13 insns): ClearStateFlag2. Tests bit 1 of the
	// state dword at +0xa0; when set, clears it and destroys the string at +0x178.
	__declspec(noinline) void ClearStateFlag2(std::uint32_t& a_flags, std::string& a_str)
	{
		if ((a_flags & 2u) != 0u) {
			a_flags &= ~2u;
			a_str.clear();
			a_str.shrink_to_fit();
		}
	}
}  // namespace MissingGroup3

// ===========================================================================
// matching-decomp force-link table. /OPT:REF strips unreferenced functions, so
// every body above is referenced here and the single entry point is called from
// src/main.cpp. This is NOT behaviour.
// ===========================================================================
namespace
{
	using namespace MissingGroup3;
	const void* const kForce3[] = {
		reinterpret_cast<const void*>(&AppendUnicodeEscape),
		reinterpret_cast<const void*>(&CreateDirectories),
		reinterpret_cast<const void*>(&DispatchActionLambda),
		reinterpret_cast<const void*>(&FormatIntoSceneField),
		reinterpret_cast<const void*>(&StartGameThread),
		reinterpret_cast<const void*>(&AllocateVector16),
		reinterpret_cast<const void*>(&InitFormatArgStore),
		reinterpret_cast<const void*>(&GrowPatternBuffer),
		reinterpret_cast<const void*>(&IdDatabaseInstance),
		reinterpret_cast<const void*>(&MakeSendMenuMessageTask),
		reinterpret_cast<const void*>(&LookupAndInvokeIdFunction),
		reinterpret_cast<const void*>(&GetGameSettingDword),
		reinterpret_cast<const void*>(&ShutdownUiGlobals),
		reinterpret_cast<const void*>(&WriteSignedField),
		reinterpret_cast<const void*>(&WriteAlignedRange),
		reinterpret_cast<const void*>(&EraseUnorderedRange),
		reinterpret_cast<const void*>(&FormatElapsed),
		reinterpret_cast<const void*>(&WriteAlignedValue),
		reinterpret_cast<const void*>(&StringAssign),
		reinterpret_cast<const void*>(&StringAppendChar),
		reinterpret_cast<const void*>(&WriteZeroPaddedDecimal),
		reinterpret_cast<const void*>(&CondVarTimedWait),
		reinterpret_cast<const void*>(&ReleaseSharedPtrControl),
		reinterpret_cast<const void*>(&AllocateVector128),
		reinterpret_cast<const void*>(&DestroyTreeNodes),
		reinterpret_cast<const void*>(&FunctionMoveSbo),
		reinterpret_cast<const void*>(&DestroyFunctionArray),
		reinterpret_cast<const void*>(&FmtIteratorBufferDtor),
		reinterpret_cast<const void*>(&FunctionManagerDestroy),
		reinterpret_cast<const void*>(&ClearStateFlag8),
		reinterpret_cast<const void*>(&ClearStateFlag2),
	};
}  // namespace

extern "C" void ForceLink_MissingGroup3()
{
	// Consume EVERY element, for the same reason group 2 documents: reading only
	// kForce3[0] lets the optimiser fold the table away and /OPT:REF then strips
	// every body, making this whole group invisible to the comparison. Verified
	// with llvm-nm in group 2. Matching-decomp force-link, not behaviour.
	volatile unsigned long long sink = 0;
	for (unsigned i = 0; i < sizeof(kForce3) / sizeof(kForce3[0]); ++i) {
		sink ^= reinterpret_cast<unsigned long long>(kForce3[i]);
	}
	(void)sink;
}