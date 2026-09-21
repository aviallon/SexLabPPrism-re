// src/missing/MissingGroup1.cpp
//
// Matching-decompilation reconstructions for the round-10 worklist group g1.
// Owner: agent `missing1`, branch grind/missing1.
//
// GROUND TRUTH CORRECTION (important for the orchestrator)
// --------------------------------------------------------
// build/recon/missing-full.csv groups *all* .pdata fragments that start at or
// after the listed address into one row, so its `orig_insn` over-counts badly
// for functions that MSVC split into several .pdata entries (C++ try/catch
// funclets). The authoritative per-function size is the .pdata entry, which is
// exactly what recon/matching.csv reports. Examples:
//
//   addr          missing-full   pdata/matching   verdict now
//   0x180047140   207            12               MISSING
//   0x18003aca0   215            65               MISSING
//   0x180054d70   51             4                MISSING (funclet head)
//   0x1800519f0   13             13               BYTE-MATCH (already paired)
//   0x180054e50   34             6                BYTE-MATCH (already paired)
//   0x18003a630   40             11               BYTE-MATCH (already paired)
//   0x18004d0f0   29             29               RATIO 1.0000 (bytes differ)
//   0x1800124d0   375            375              RATIO 0.9800 (first_div 44)
//
// Real g1 work is therefore ~1000 instructions, not ~2600. Of those, many are
// the heads/handlers of funclet-split STL/format functions that cannot be
// reproduced as standalone bodies without the outer function's exact try/catch
// shape; they are reported as blocked rather than duplicated here.
//
// BODIES BELOW
// ------------
// The bodies below are faithful structural reconstructions of complete,
// standalone .pdata functions from g1, written against real MSVC types so the
// compiler emits the same call/dtor structure. Names are GUESSES derived from
// behaviour; the comment records the address and the observed structure.
//
// These are matching-decompilation force-link stubs, NOT behaviour the plugin
// needs. The single hook to add to src/main.cpp is:
//
//     extern "C" void ForceLink_MissingGroup1();
//     ...
//     ForceLink_MissingGroup1();

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <string>
#include <vector>

#if defined(_MSC_VER)
#include <intrin.h>
#else
extern "C" long _InterlockedExchangeAdd(volatile long*, long);
#endif

namespace {

// ---------------------------------------------------------------------------
// 0x1800107a0 (22 insns, 70 bytes .pdata range 0x1800107a0-0x1800107e6)
// GUESS: ReleaseRefBlock::operator()  -- destroy a ref-counted block held by
// pointer-to-pointer. oracle:
//   b = *holder; if (!b) return;
//   r = *b; if (r) { if (lock xadd [r],-1 == 1) operator delete(r, 0x20); }
//   operator delete(b, 0x10);            <- tail jump, sized delete
// ---------------------------------------------------------------------------
void ReleaseRefBlock_1800107a0(void** holder) {
    void* block = *holder;
    if (block == nullptr) {
        return;
    }
    long* ref = *reinterpret_cast<long**>(block);
    if (ref != nullptr) {
        if (_InterlockedExchangeAdd(ref, -1) == 1) {
            operator delete(ref, 0x20);
        }
    }
    operator delete(block, 0x10);
}

// ---------------------------------------------------------------------------
// 0x180052f70 (13 insns, 47 bytes 0x180052f70-0x180052f9f)
// GUESS: CleanupFlaggedString::run  -- SEH cleanup for a frame slot: if the
// "constructed" bit (2) is set in the flags at +0x90, clear it and destroy the
// std::string at +0x98. rcx is unused in the oracle.
// ---------------------------------------------------------------------------
struct FlaggedStringSlot_180052f70 {
    uint8_t pad0[0x90];
    uint32_t flags;    // 0x90
    uint32_t pad1;     // 0x94
    std::string str;   // 0x98
};

void CleanupFlaggedString_180052f70(void* /*unused*/, FlaggedStringSlot_180052f70* slot) {
    if (slot->flags & 2u) {
        slot->flags &= ~2u;
        slot->str.~basic_string();
    }
}

// ---------------------------------------------------------------------------
// 0x1800519f0 (13 insns, 38 bytes 0x1800519f0-0x180051a16) -- ALREADY BYTE-MATCH
// (matching.csv bytematch=1, anchor=icf). Included only for completeness.

// ---------------------------------------------------------------------------
// 0x180016820 (16 insns, 53 bytes 0x180016820-0x180016855)
// GUESS: DestroyingDtor::scalar_deleting_dtor(Obj*, char deleting)
//   destroys the std::string at +8, then (if deleting) operator delete(this,0x30).
// ---------------------------------------------------------------------------
struct DtorObj_180016820 {
    void* vtbl;        // 0x00
    std::string str;   // 0x08
    uint8_t tail[0x30 - 0x08 - sizeof(std::string)];
};

void DeletingDtor_180016820(DtorObj_180016820* self, char deleting) {
    self->str.~basic_string();
    if (deleting) {
        operator delete(self, 0x30);
    }
}

// ---------------------------------------------------------------------------
// 0x180046db0 (14 insns pdata head; body continues past the entry)
// GUESS: vector<void*>::assign(n, value) out-of-line body: grow to n*8 bytes,
// value-fill, else fill existing range. Written as the real std::vector call so
// MSVC emits its own _Assign_counted instantiation.
// ---------------------------------------------------------------------------
void VectorAssignVoidPtr_180046db0(std::vector<void*>* v, size_t n, void* value) {
    v->assign(n, value);
}

// ---------------------------------------------------------------------------
// 0x18003a630 (11 insns, 35 bytes 0x18003a630-0x18003a653) -- ALREADY BYTE-MATCH
// (matching.csv bytematch=1, anchor=icf). std::string assign/append helper.

}  // namespace

// ---------------------------------------------------------------------------
// Matching-decomp force-link table. /Gy + /OPT:REF would strip these bodies
// because nothing calls them; main.cpp calls ForceLink_MissingGroup1() to keep
// them (and their vtables/helpers) in the object.
// ---------------------------------------------------------------------------
namespace {
const void* const kForceMissingGroup1[] = {
    reinterpret_cast<const void*>(&ReleaseRefBlock_1800107a0),
    reinterpret_cast<const void*>(&CleanupFlaggedString_180052f70),
    reinterpret_cast<const void*>(&DeletingDtor_180016820),
    reinterpret_cast<const void*>(&VectorAssignVoidPtr_180046db0),
};
}  // namespace

extern "C" void ForceLink_MissingGroup1() {
    volatile const void* sink = kForceMissingGroup1[0];
    (void)sink;
}
