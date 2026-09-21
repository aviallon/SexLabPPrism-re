// MissingRound2Group1.cpp — round-2 (relaunch) reconstructions for the MISSING
// real functions with group == "g1" in build/recon/missing-full.csv.
//
// OWNER: missing4 (grind/missing4). Decompiled evidence:
// build/recon/decompiled-g1r2/0x<addr>_*.c (Ghidra 12.1.2, 20/20 clean; see
// recon/DECOMPILE-NOTES.md for the export command). Names are GUESSES recovered
// from behaviour (calls, literals, structures) unless a vftable/__FUNCSIG__
// literal pins them; each comment carries the address and the instruction count.
//
// PRESENT-BUT-UNPAIRED (NOT duplicated here — a body already exists in src/):
//   0x1800261e0  `anonymous-namespace'::ApplyPresentation  src/Presentation.cpp:143
//   0x18002d340  `anonymous-namespace'::JsCatalogDone      src/UiBridge.cpp:51
//   0x18002bea0  `anonymous-namespace'::QueuePresentation  src/Presentation.cpp:115
//   0x18000fef0  StartGameThread                           src/missing/MissingGroup3.cpp:123
//   0x1800382b0  GetGameSettingDword                       src/missing/MissingGroup3.cpp:209
//   0x180044650  WriteZeroPaddedDecimal                    src/missing/MissingGroup3.cpp:338
//   0x180046f00  AllocateVector128                         src/missing/MissingGroup3.cpp:374
//   0x18002dda0  DestroyFunctionArray                       src/missing/MissingGroup3.cpp:399
//   0x180053420  ClearStateFlag2                            src/missing/MissingGroup3.cpp:435
//   0x180040890  mg2_FillWords                             src/missing/MissingGroup2.cpp:41
//   0x180037490  mg2_IdLookupA                             src/missing/MissingGroup2.cpp:284
//   0x180054d70  (no body anywhere)                        — implemented below
//
// FORCE LINK: /Gy + /OPT:REF strips any body nothing references. The table at
// the bottom consumes every element so the optimiser cannot fold it away; a
// call to ForceLink_Missing4Group1() from a live path (src/main.cpp) is
// required. This is a matching-decomp artefact, NOT behaviour.

#include "MissingRound2Group1.h"

#include "PCH.h"

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <exception>
#include <format>
#include <functional>
#include <mutex>
#include <new>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace
{
	// -----------------------------------------------------------------------
	// 0x180054d70 (library, 51 insns in missing-full; real .pdata 4) —
	// ShutdownCaptionStore. Iterates the caption/string array
	// [0x18009c1d8, 0x18009c1e0) in 0x10-byte strides, destroying four
	// std::string members per element, then frees the backing block
	// (0x18009c1d8) and zeroes begin/end/capacity. Reproduced with an explicit
	// destructor loop so the body exists standalone; the original is a
	// compiler-generated `vector<S>::~vector` tail.
	// -----------------------------------------------------------------------
	struct CaptionSlot
	{
		std::string a;
		std::string b;
		std::string c;
		std::string d;
	};

	__declspec(noinline) void ShutdownCaptionStore(std::vector<CaptionSlot>& a_store)
	{
		for (auto& slot : a_store) {
			slot.d.~basic_string();
			slot.c.~basic_string();
			slot.b.~basic_string();
			slot.a.~basic_string();
		}
		a_store.clear();
		a_store.shrink_to_fit();
	}

	// -----------------------------------------------------------------------
	// 0x180023230 (library, 117 insns) — GUESS: ConcatStrings, the MSVC
	// `std::string operator+(const string&, const string&)` / `_Reallocate_
	// grow_by` core. Reads the two source lengths, chooses between reuse
	// (append into an existing buffer) and fresh allocation with the max(15,
	// len) capacity curve, then memcpy's both payloads and NUL-terminates.
	// Expressed as a real operator+ so the compiler emits that same core.
	// -----------------------------------------------------------------------
	__declspec(noinline) std::string ConcatStrings(const std::string& a_left, const std::string& a_right)
	{
		std::string out;
		out.reserve(a_left.size() + a_right.size());
		out.append(a_left);
		out.append(a_right);
		return out;
	}

	// -----------------------------------------------------------------------
	// 0x180006290 (plugin/library, 42 insns) — GUESS: ReleaseThreadControlBlock,
	// a std::thread / future control-block teardown. Holds a pointer to an
	// atomic refcount: increments, runs the completion callback, decrements and
	// frees at zero; issues the thread-exit condition-variable broadcast
	// (_Cnd_do_broadcast_at_thread_exit) and finally frees the block itself.
	// -----------------------------------------------------------------------
	struct ThreadControlBlock
	{
		std::atomic<std::int32_t>* refs;
		std::int64_t               callback;
	};

	__declspec(noinline) void ReleaseThreadControlBlock(ThreadControlBlock* a_block)
	{
		std::atomic<std::int32_t>* refs = a_block->refs;
		a_block->refs                    = nullptr;
		if (refs == nullptr) {
			// original tail-calls FUN_180013a20(callback, &refs): the scheduler
			// poke that wakes a waiter; a no-op here (no runtime path).
		} else {
			refs->fetch_add(1, std::memory_order_relaxed);
			if (refs->fetch_sub(1, std::memory_order_acq_rel) == 1) {
				std::free(refs);
			}
		}
		// _Cnd_do_broadcast_at_thread_exit()
		std::atomic_thread_fence(std::memory_order_acq_rel);
		if (a_block->refs != nullptr) {
			if (a_block->refs->fetch_sub(1, std::memory_order_acq_rel) == 1) {
				std::free(a_block->refs);
			}
		}
		std::free(a_block);
	}

	// -----------------------------------------------------------------------
	// 0x180013f20 (plugin, 99 insns) — GUESS: ScheduleDelayedTask. Locks the
	// queue mutex, rejects a saturated queue (0x7fffffff -> Cpp_error(6)),
	// computes `now + millis*1000000`, moves the std::function into an entry,
	// pushes it on the deque at +0x98 and broadcasts the condition variable at
	// +0x50. The time base comes from FUN_18001d7d0.
	// -----------------------------------------------------------------------
	struct DelayedTaskQueue
	{
		std::mutex                                              mtx;
		std::condition_variable                                 cv;
		std::deque<std::pair<std::int64_t, std::function<void()>>> tasks;
		std::uint32_t                                           scheduled = 0;

		std::int64_t now() const { return 0; }
	};

	__declspec(noinline) void ScheduleDelayedTask(DelayedTaskQueue& a_queue, std::int64_t a_millis,
		std::function<void()> a_task)
	{
		if (!a_task) {
			return;
		}
		std::unique_lock<std::mutex> lock(a_queue.mtx);
		if (a_queue.scheduled == 0x7fffffffu) {
			throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again));
		}
		const std::int64_t deadline = a_queue.now() + a_millis * 1000000;
		a_queue.tasks.emplace_back(deadline, std::move(a_task));
		a_queue.scheduled += 1;
		lock.unlock();
		a_queue.cv.notify_all();
	}

	// -----------------------------------------------------------------------
	// 0x18002c9d0 (library, 16 insns) — GUESS: CatalogAppendFunction copy
	// constructor, the MSVC `std::_Func_impl_no_alloc<Papyrus_CatalogAppend
	// lambda_1, void>` copy ctor: installs the lambda vftable, copies the
	// 8-byte capture, the 4-byte capture and the captured std::string.
	// -----------------------------------------------------------------------
	struct CatalogAppendFunction
	{
		const void*  vftable;
		std::int64_t capture0;
		std::int32_t capture1;
		std::string  text;

		__declspec(noinline) CatalogAppendFunction(const CatalogAppendFunction& a_other)
			: vftable(a_other.vftable), capture0(a_other.capture0), capture1(a_other.capture1),
			  text(a_other.text)
		{
		}
	};

	// Free wrapper so the copy ctor's address can be forced into the table.
	__declspec(noinline) CatalogAppendFunction* CopyCatalogAppendFunction(
		CatalogAppendFunction* a_dst, const CatalogAppendFunction& a_src)
	{
		return ::new (a_dst) CatalogAppendFunction(a_src);
	}

	// -----------------------------------------------------------------------
	// 0x1800374f0 (plugin, 20 insns) and 0x1800390b0 (plugin, 21 insns) —
	// REL::IDDatabase lookups returning a dereferenced address. Three-word ID
	// literals are resolved via id2offset (REL::IDDatabase::id2offset), the
	// module base comes from +0x110 of the resolved entry
	// (FUN_18001bc40), and the second id2offset result indexes it. GUESS:
	// REL::Relocation<...>::address for two Address-Library ids.
	//   0x1800374f0 literals: {0x7e3b3, 0x6285f, 0x7e3b3}
	//   0x1800390b0 literals: the .rdata triple at 0x1800615a0
	// -----------------------------------------------------------------------
	struct RelIdTriple
	{
		std::uint64_t ids[3];
	};

	std::uintptr_t Id2Offset(const RelIdTriple& a_ids)
	{
		(void)a_ids;
		return 0;  // REL::IDDatabase::id2offset shim (no runtime path)
	}

	void* ResolveEntry(const RelIdTriple& a_ids)
	{
		(void)a_ids;
		return nullptr;  // FUN_18001bc40 shim
	}

	__declspec(noinline) std::uintptr_t IdLookup_7e3b3()
	{
		static const RelIdTriple kIds{ { 0x7e3b3ull, 0x6285full, 0x7e3b3ull } };
		if (Id2Offset(kIds) == 0) {
			return 0;
		}
		auto* const          entry = static_cast<std::uint8_t*>(ResolveEntry(kIds));
		const std::uintptr_t base  = *reinterpret_cast<std::uintptr_t*>(entry + 0x110);
		const std::uintptr_t off   = Id2Offset(kIds);
		return *reinterpret_cast<std::uintptr_t*>(base + off);
	}

	__declspec(noinline) std::uintptr_t IdLookup_615a0()
	{
		static const RelIdTriple kIds{ { 0ull, 0ull, 0ull } };  // 0x1800615a0 triple
		if (Id2Offset(kIds) == 0) {
			return 0;
		}
		auto* const          entry = static_cast<std::uint8_t*>(ResolveEntry(kIds));
		const std::uintptr_t base  = *reinterpret_cast<std::uintptr_t*>(entry + 0x110);
		const std::uintptr_t off   = Id2Offset(kIds);
		return *reinterpret_cast<std::uintptr_t*>(base + off);
	}

	// -----------------------------------------------------------------------
	// 0x18003e6e0 (library, 115 insns) — GUESS: FormatAlignedField, the MSVC
	// std::format fill/alignment writer. Reads width from the format spec,
	// the align byte at +9 (defaulting to the caller's param 5), the fill run
	// at +0xf of length +0xe; writes the fill bytes (left or split by the
	// computed left/right counts) then the value into the output buffer.
	// Surfaced as a real std::format call with `fill`/`align`.
	// -----------------------------------------------------------------------
	__declspec(noinline) std::string FormatAlignedField(std::int32_t a_value, std::int32_t a_width)
	{
		return std::format("{:*>{}}", a_value, a_width);
	}

	// -----------------------------------------------------------------------
	// 0x180044ab0 (library, 88 insns) — GUESS: MoveBlockStore, the MSVC
	// std::deque move constructor. Copies the value kind, allocates a fresh
	// 0x20-byte sentinel node and self-links it, zeros the proxy/begin/end
	// slots, steals the source map+size handles, then swaps the 7/8 block-size
	// fields with the source (which is left empty). Reproduced as an explicit
	// sentinel+steal move so the standalone body exists.
	// -----------------------------------------------------------------------
	struct BlockStore
	{
		std::int32_t  kind;
		std::uint32_t pad;
		void*         map;
		void*         proxy;
		void*         first;
		void*         last;
		void*         end;
		std::uint32_t map_size;
		std::uint32_t block_size;
	};

	__declspec(noinline) void MoveBlockStore(BlockStore& a_dst, BlockStore& a_src)
	{
		a_dst.kind = a_src.kind;
		a_dst.map  = nullptr;
		a_dst.proxy = nullptr;
		void** const sentinel = static_cast<void**>(::operator new(0x20));
		sentinel[0]           = sentinel;
		sentinel[1]           = sentinel;
		a_dst.first          = sentinel;
		a_dst.last           = sentinel;
		a_dst.end            = sentinel;
		a_dst.map_size       = 0;
		a_dst.block_size     = 0;

		void* const map       = a_dst.map;
		a_dst.map             = a_src.map;
		a_src.map             = map;
		void* const proxy     = a_dst.proxy;
		a_dst.proxy           = a_src.proxy;
		a_src.proxy           = proxy;
		void* const first     = a_dst.first;
		a_dst.first           = a_src.first;
		a_src.first           = first;
		void* const last      = a_dst.last;
		a_dst.last            = a_src.last;
		a_src.last            = last;
		void* const end       = a_dst.end;
		a_dst.end             = a_src.end;
		a_src.end             = end;
		const std::uint32_t ms = a_dst.map_size;
		a_dst.map_size        = a_src.map_size;
		a_src.map_size        = 7;
		const std::uint32_t bs = a_dst.block_size;
		a_dst.block_size      = a_src.block_size;
		a_src.block_size      = 8;
		(void)ms;
		(void)bs;
	}

	// -----------------------------------------------------------------------
	// The force-link table. Every element is observed by the loop below so the
	// optimiser cannot constant-fold the unused ones away (the /OPT:REF trap).
	// -----------------------------------------------------------------------
	const void* const kForce4[] = {
		reinterpret_cast<const void*>(&ShutdownCaptionStore),
		reinterpret_cast<const void*>(&ConcatStrings),
		reinterpret_cast<const void*>(&ReleaseThreadControlBlock),
		reinterpret_cast<const void*>(&ScheduleDelayedTask),
		reinterpret_cast<const void*>(&CopyCatalogAppendFunction),
		reinterpret_cast<const void*>(&IdLookup_7e3b3),
		reinterpret_cast<const void*>(&IdLookup_615a0),
		reinterpret_cast<const void*>(&FormatAlignedField),
		reinterpret_cast<const void*>(&MoveBlockStore),
	};
}  // namespace

extern "C" void ForceLink_Missing4Group1()
{
	// Read every element: observing only kForce4[0] lets the optimiser drop the
	// rest of the table and the linker then strips those bodies.
	volatile std::uintptr_t sink = 0;
	for (std::size_t i = 0; i < sizeof(kForce4) / sizeof(kForce4[0]); ++i) {
		sink ^= reinterpret_cast<std::uintptr_t>(kForce4[i]);
	}
	(void)sink;
}