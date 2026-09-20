# SexLabPPrism 0.6.1 — reconstruction

A from-binary reconstruction of the closed-source **SexLab P+ Prism** SKSE
plugin (`SexLabPPrism.dll` 0.6.1), rebuilt as source so it can be recompiled
against current CommonLibSSE-NG and read Address Library **format 5**
(`Data/SKSE/Plugins/versionlib-1-7-104-0.bin`), which the shipped 0.6.1 DLL
rejects on Skyrim AE 1.7.104 with:

```
[critical] failed to open address library file
```

This repository is **not** the original source. The original is not published
anywhere; everything here was recovered from the shipped artifacts by earlier
recon passes that read the compiled `.pex` scripts and the DLL's
exports/strings/RTTI/decompilation. Those recon notes stay in the local
project workspace and are deliberately not published here, because they quote
the original binary's internals. This is an unofficial reconstruction, not a
byte-for-byte reproduction.

## What is implemented

The **plugin surface**, exactly as the original exposes it:

| SKSE export | Original evidence | Reconstruction |
|---|---|---|
| `SKSEPlugin_Query` | `infoVersion = 1`, `name = "SexLabPPrism"`, `version = 0x00060010` | `src/main.cpp` — same values, no runtime check |
| `SKSEPlugin_Version` | data: `dataVersion=1`, `pluginVersion=0x00060010`, `name`, **no `compatibleVersions` table** | `src/main.cpp` — `constinit SKSE::PluginVersionData`, independence flags zeroed |
| `SKSEPlugin_Load` | `SKSE::Init` → banner → Papyrus register | `src/main.cpp` — `SKSE::Init`, version banner, native registration |

The plugin creates its log file (`Documents/My Games/Skyrim Special
Edition/SKSE/SexLabPPrism.log`, named from `PluginVersionData::pluginName` by
CommonLibSSE-NG) and logs the same version banner text as the original
("SexLab P+ Prism 0.6.1 loading (strict lifecycle + confirmed freecam +
selective UI)").

The **13 Papyrus natives** of the hidden `SexLabPrismNative` script are
registered with the exact names and argument types recovered from
`SexLabPrismNative.pex` (`src/Papyrus/Natives.cpp`):

```
Log  BeginSceneSession  PublishSceneState  IsFreeCameraActive
PublishCompatible  CatalogBegin  CatalogAppend  CatalogPackage  CatalogFinish
IsCatalogReady  GetCatalogCount  CatalogPublish  SetSearchQuery
```

## What is stubbed

Behaviour is only partly recovered so far. A decompilation pass recovered
pseudocode for all 13 natives — 11 of them need nothing beyond fmt/STL/mutex
plus the SKSE `TaskInterface`, and only `IsFreeCameraActive` and the two event
sinks touch real engine types (`RE::PlayerCamera`, `BSTEventSink<RE::InputEvent*>`) —
but reimplementing them is the *next* step, not this one. Every native except
`Log` therefore logs

```
<name>: not implemented yet
```

and returns a safe default:

| native | stub return |
|---|---|
| `BeginSceneSession` | `0` (no session) |
| `IsFreeCameraActive` | `false` |
| `IsCatalogReady` | `false` |
| `GetCatalogCount` | `0` |
| all `void` natives | no-op + warning |

`Log(String)` is implemented (it forwards the message to the plugin log); it is
the one native whose behaviour is fully determined by the contract.

The PrismaUI web-view, the JSON scene/catalogue protocol, the event sinks
(`InputSink`, `MenuVisibilitySink`), `FocusRecovery` and the freecam/HUD logic
of the original are **not** reconstructed in this step.

## Build

The build uses xmake + C++23 + CommonLibSSE-NG, following the conventions of
the upstream SexLabpp project.

```bash
git clone --depth 1 --branch ng https://github.com/alandtse/CommonLibVR.git lib/CommonLibSSE-NG
python tools/check_commonlib.py lib/CommonLibSSE-NG   # asserts format-5 support
xmake f -p windows -a x64 -m release -y
xmake build SexLabPPrism
```

`.github/workflows/build.yml` (windows-latest) does this from scratch, but it
resolves the **latest** `ng` HEAD dynamically — per the project rule "use the
LATEST CommonLibSSE-NG, never an older revision" — and asserts before building:

* `include/REL/IDDB.h` exists,
* it contains `SSEv5` **inside `enum class Format`**,
* it declares `load_v5`.

`tools/check_commonlib.py` performs that assertion and prints the resolved
revision; the workflow fails loudly if any check fails. It also takes an
optional pinned ref via `workflow_dispatch`.

The packaged artifact is `SexLabPPrism-0.6.1-1.7.104.zip` with
`SKSE/Plugins/SexLabPPrism.dll` at the **archive root** plus a
`README-PORT.txt`, so Amethyst's "Mods → Install mod from file…" installs it
without manual file placement.

## Verification story

* Proven locally (Linux): the three exports are defined in `src/main.cpp`;
  the native table is exactly 13 names (compile-time `static_assert` in
  `src/Papyrus/Natives.h`); `check_commonlib.py` passes against a real
  CommonLibSSE-NG checkout; `xmake.lua` and `build.yml` parse.
* Requires CI (Windows + MSVC): compilation into a 64-bit DLL, MSVC accepting
  the CommonLibSSE-NG headers at the resolved revision, and the packaged zip.
* Requires a live Skyrim AE 1.7.104 install: that SKSE loads the DLL, that
  `SexLabPPrism.log` is created with the banner, and that the native surface
  resolves for the Prism Papyrus scripts. Until then the natives are stubs by
  design and Prism's UI behaviour is expected not to work.

Honest limitation: this step was about compiling and loading. It does **not**
claim functional parity with the original Prism.