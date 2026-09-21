# spdlog / STL instantiation inventory — original vs our parity build

Agent: `spdlogshape` (branch `grind/spdlogshape`). Data generated from the
binaries; every number below is reproducible with the commands in
"Reproduce" at the end.

| image | path | sha256(first32) | size |
|---|---|---|---|
| ORIG (the ROM) | `/home/aviallon/Projects/SexLabPPrism-re/artifacts/SexLabPPrism.dll` | `9da2bfa889a53fe6d2f36ce8f4388f46` | 656,384 |
| PAR (parity) | `/home/aviallon/Projects/SexLabPPrism-re/artifacts/rebuild/parity-r14revert.dll` | see `recon/rtti-PAR.txt` | 795,136 |

Method: `tools/pe_rtti.py <dll>` recovers MSVC type descriptors (TD), COLs and
vtables; `tools/spdlog_inventory.py` classifies the spdlog TDs. RTTI is the
right evidence here because spdlog emits no `__FUNCSIG__` literals: the literal
scan finds **0** spdlog `__cdecl`/`__fastcall` signatures in either image, so
the only typed names available are TD names and vtable-slot symbols.

## 1. The original's spdlog type set (96 TDs)

| family | count | evidence |
|---|---|---|
| flag formatter templates, `null_scoped_padder` | 35 | `.??$<f>_formatter@Unull_scoped_padder@details@spdlog@@@details@spdlog@@` |
| flag formatter templates, `scoped_padder` | 35 | `.??$<f>_formatter@Vscoped_padder@...` |
| `elapsed_formatter<Padder, duration>` | 8 | 4 units (`$00$00`=seconds, `$00$0DOI`=ms, `$00$0PECEA`=us, `$00$0DLJKMKAA`=ns) × 2 padders |
| non-template formatters | 5 | `aggregate_formatter`, `ch_formatter`, `color_start_formatter`, `color_stop_formatter`, `full_formatter` |
| formatter bases | 3 | `flag_formatter`, `formatter`, `pattern_formatter` |
| `logger` (+ `_Ref_count_obj2<logger>`) | 2 | `.??AVlogger@spdlog@@`, `.?AV?$_Ref_count_obj2@Vlogger@spdlog@@@std@@` |
| sinks | 7 | `sink@sinks`, `base_sink<mutex>`, `basic_file_sink<mutex>` + ref, `wincolor_sink<console_mutex>`, `wincolor_stdout_sink<console_mutex>` + ref |
| `spdlog_ex` | 1 | `.?AVspdlog_ex@spdlog@@` |
| **total** | **96** | |

The 35 distinct flag templates present (same 35 for both padders):
`a A b B c C d D e E f F H I m M mdc name p pid r R short_filename
short_level source_filename source_funcname source_linenum source_location
t T v Y z` — i.e. **the full spdlog 1.16 flag set**, including flags
(`o O i u` elapsed, `&` mdc, `!` funcname, `%` ch) that do **not** occur in the
original's pattern string.

## 2. Our parity build's set (98 TDs)

Identical to the original, **plus two**:

| only in PAR | evidence |
|---|---|
| `.??$msvc_sink@Vmutex@std@@@sinks@spdlog@@` | TD present in PAR, absent in ORIG |
| `.?AV?$_Ref_count_obj2@V?$msvc_sink@Vmutex@std@@@sinks@spdlog@@@std@@` | ditto |

## 3. Diff

```
spdlog TDs: ORIG=96 PAR=98  only-ORIG=0  only-PAR=2
  ONLY-PAR .?AV?$msvc_sink@Vmutex@@@sinks@spdlog@@
  ONLY-PAR .?AV?$_Ref_count_obj2@V?$msvc_sink@...@@@std@@
```

**There is no spdlog type the original has that we lack.** Every formatter
(including all 8 `elapsed_formatter` classes), every sink, `pattern_formatter`
and `logger` exist in both. The only difference is the *extra* CLNG
`msvc_sink`, which is **present-without-a-partner**, not missing.

## 4. Pattern strings — byte-for-byte

| image | pattern literals found |
|---|---|
| ORIG | `[%Y-%m-%d %H:%M:%S.%e] [%l] %v` |
| PAR | `[%Y-%m-%d %H:%M:%S.%e] [%l] %v`  **and**  `[%T.%e] [%=5t] [%L] %v` |

Our `SetupLog` pattern is **character-for-character identical** to the
original's. The second string in PAR is CLNG's `log::init()/msvc_sink` default,
which only exists because our build still links CLNG's logging code — the
original's whole-program build eliminated it.

## 5. Mechanism — why the pattern does NOT control the formatter set

spdlog 1.16 `include/spdlog/pattern_formatter-inl.h`:

* `handle_flag_` is a **member function template** `template<typename Padder>`.
  Its body is a `switch (flag)` whose case bodies each call
  `make_unique<...formatter<Padder>>`. In C++, **every case body is compiled**,
  so every referenced template is instantiated at compile time; the runtime
  pattern only chooses which `make_unique` executes.
* `compile_pattern_` calls it for **both** padders unconditionally:
  ```cpp
  if (padding.enabled())
      handle_flag_<details::scoped_padder>(*it, padding);
  else
      handle_flag_<details::null_scoped_padder>(*it, padding);
  ```
  hence the pairs of identical counts (35 null / 35 scoped, 4×2 elapsed).
* The elapsed families come from the `case 'u'/'i'/'o'/'O'` bodies
  (`nanoseconds/microseconds/milliseconds/seconds`) — again instantiated
  whether or not the runtime pattern contains `%o`.

**Consequence:** the set of instantiated formatters is a property of *spdlog
being compiled into this image*, not of the pattern string. Changing
`SetupLog`'s pattern cannot add or remove a single formatter instantiation.
The observation that the original contains `elapsed_formatter` while its
pattern has no elapsed flag is itself the proof of switch-instantiation.

Sinks are decided by the sink objects `SetupLog` constructs:
`basic_file_sink_mt` + `wincolor_stdout_sink_mt` (+ their `base_sink`/`sink`
bases) — identical in both. `SPDLOG_USE_STD_FORMAT` is already settled: neither
image contains any `fmt::` symbol, and both use `wincolor_stdout_sink`.

## 6. The two "MISSING" spdlog rows are a matcher artifact, not absence

`recon/matching.csv` (generated against `parity-r14revert.dll`) reports two
library-tier MISSING rows among the 117 spdlog formatter/sink rows:

| orig addr | name | verdict | true status in PAR |
|---|---|---|---|
| `0x18004a450` | `elapsed_formatter<scoped_padder, milliseconds>::vfunc` | MISSING | **present** at `0x18005b060` (disassembled; same body shape) |
| `0x180044ca0` | `.??AVlogger@spdlog@@::vtable[0]` (rtti-ref-site name, 126 insns) | MISSING | body present; opcode-identical candidates exist in the spdlog region, matcher could not disambiguate |

The `elapsed_formatter` clone is at PAR `0x18005b060` — the class's vtable slot
read directly from `recon/rtti-PAR.json` points at it and `objdump` shows a
normal function prologue. `match.py --focus` cannot see it because the address
is not a separately-inventoried `.pdata` start (fragment/chained-unwind
handling), and because many spdlog `clone()` bodies have identical opcode
sequences. This is the **pairing** problem from round 1, not a missing
instantiation.

The shapes also differ slightly (e.g. ORIG prologue `push rbx,rbp,rsi,...; sub
$0x50` vs PAR `mov [rsp+10],rbx; mov [rsp+18],rbp; push rsi,...; sub $0x60`),
and the trivial null_padder clones differ by one register byte at instruction 1
(`4c 8b 52 18` vs `48 8b 42 18`) — a register-allocation/codegen difference,
which is a build property, not a source configuration.

## 7. Verdict

* **Configuration differences in `SetupLog`: none.** Pattern is identical;
  sink set is identical; formatter/sink/logger instantiation sets are
  identical. There is nothing fixable in `SetupLog` that would add a missing
  type.
* **The remaining spdlog deltas are PACKAGING:**
  * our build has an *extra* `msvc_sink` + CLNG `log::init` (unreachable code
    the original's whole-program LTCG eliminated; we cannot reach it from
    `SetupLog` because SKSE::Init is a separate, non-LTCG object);
  * our spdlog is a separate non-`/GL` static library, so its function bodies
    are not LTCG-merged/optimised in the original's whole-program context, and
    their shapes differ (and the pairer then fails to pair them).
* The fix, if pursued, is to vendor/build spdlog **inside** our LTCG unit with
  the same options (`header_only=false, wchar=true, std_format=true`) and
  round the whole program — but round 1 measured that `PRISM_LTO_SCOPE=clng` /
  `project` collapses matching (846 → 527 real-function BYTE), so this must be
  an isolated spdlog-only experiment, not a repeat of that knob.

## 8. Prediction if a build is requested

No source change is proposed from this analysis. If a **measurement-only**
re-run of `tools/match.py` (r14 artifact, unchanged) with pairing that resolves
the spdlog clones were done:

* types that would appear: none new (all 96 already present);
* spdlog library MISSING rows that would clear: **2** (`0x18004a450`,
  `0x180044ca0`), and they would clear to **RATIO / INSN**, not BYTE, because
  the prologue/register-allocation shapes genuinely differ;
* BYTE on the library tier: **0 gained**;
* risk to the shipping configuration: none (no build, no source change).

## Reproduce

```sh
cd /tmp && mkdir -p recon && cp <repo>/tools/pe_rtti.py .
python3 pe_rtti.py <ORIG.dll>   # -> recon/rtti.txt, recon/rtti.json
mv recon/rtti.txt recon/rtti-ORIG.txt; mv recon/rtti.json recon/rtti-ORIG.json
python3 pe_rtti.py <PAR.dll>
mv recon/rtti.txt recon/rtti-PAR.txt; mv recon/rtti.json recon/rtti-PAR.json
python3 <repo>/tools/spdlog_inventory.py recon/rtti-ORIG.json
python3 <repo>/tools/spdlog_inventory.py recon/rtti-PAR.json
python3 <repo>/tools/spdlog_inventory.py --diff recon/rtti-ORIG.json recon/rtti-PAR.json
```