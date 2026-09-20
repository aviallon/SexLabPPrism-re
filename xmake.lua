-- SexLabPPrism 0.6.1 reconstruction — Windows x64 SKSE plugin.
--
-- Mirrors the build conventions of the upstream SexLabpp project
-- (xmake + CommonLibSSE-NG, C++23, MSVC runtime MD). The CommonLibSSE-NG
-- submodule lives at lib/CommonLibSSE-NG and is pulled by the CI workflow at
-- the LATEST alandtse/CommonLibVR `ng` HEAD; tools/check_commonlib.py asserts
-- that revision has include/REL/IDDB.h with the format-5 `Format::SSEv5`
-- reader before anything is built.
set_xmakever("3.0.0")

-- Globals
PROJECT_NAME = "SexLabPPrism"
PROJECT_VERSION = "0.6.1"

-- ---------------------------------------------------------------------------
-- Dependency pins for parity sweeps.
--
-- The CLNG revision this project builds against (v4.38.0,
-- 1cc6b3999eb92b4d86be08be28011ba679980081) hardcodes its own package versions
-- inside lib/CommonLibSSE-NG/xmake.lua:
--
--   add_requires("directxmath 2024.02", "directxtk 24.2.0")
--   add_requires("spdlog v1.16.0",
--                { configs = { header_only = false, wchar = true,
--                              std_format = true } })
--   add_requires("rapidcsv v8.92")
--
-- Those are recovered (not guessed) versions: the sibling upstream build
-- SexLabpp/xmake-requires.lock resolves exactly directxmath 2024.02,
-- directxtk 24.2.0, rapidcsv v8.92 and spdlog v1.16.0 against xmake-repo
-- commit 33a3d2592b35c2e02b01b0824a9f017a787ddec1, and the original DLL
-- contains no fmt symbols at all, which is only true with spdlog's
-- std_format=true configuration.  A transitive add_requires cannot be
-- re-pointed from the command line, so each PRISM_<DEP>_VERSION environment
-- variable (set by the workflow_dispatch inputs) becomes an add_requireconfs()
-- override.  An empty variable keeps CLNG's own pin, so the normal parity job
-- is unchanged and the pinned-era gate still runs.
--
-- add_requireconfs() must run BEFORE includes() pulls in CLNG's script, since
-- that script is where add_requires() for these packages is executed.
-- ---------------------------------------------------------------------------
local function prism_pin(dep, version, conf)
    if version and version ~= "" then
        conf = conf or {}
        conf.override = true
        conf.version = version
        add_requireconfs(dep, conf)
        print(string.format("[prism] dependency override: %s = %s", dep, version))
    end
end

prism_pin("spdlog", os.getenv("PRISM_SPDLOG_VERSION"), {
    configs = { header_only = false, wchar = true, std_format = true },
})
prism_pin("directxmath", os.getenv("PRISM_DIRECTXMATH_VERSION"))
prism_pin("directxtk", os.getenv("PRISM_DIRECTXTK_VERSION"))
prism_pin("rapidcsv", os.getenv("PRISM_RAPIDCSV_VERSION"))

-- CommonLibSSE-NG (alandtse/CommonLibVR `ng`). Must exist before this script is
-- parsed; the workflow checks it out first, and tools/check_commonlib.py proves
-- it is a format-5-capable revision.
--
-- LTCG scope knob (matching experiment, 2026-09-21). The original's Rich header
-- carries Utc1920_LTCG_CPP/_C and CLNG's CMake sets CMAKE_INTERPROCEDURAL_
-- OPTIMIZATION=ON for Release, so SOMETHING in the original was built with /GL and
-- linked /LTCG. What is NOT yet established is whether the author's own plugin
-- translation units (main.cpp, the sinks, ...) were /GL too: original
-- InputSink::ProcessEvent (0x18001f0b0) still makes a real `call` to
-- FocusRecovery::Begin/Cancel, while our target-scoped /GL inlines
-- FocusRecovery::Begin into InputSink (sub-32). Both are consistent with "CLNG
-- internals were LTCG-merged, the plugin TUs were not". PRISM_LTO_SCOPE selects
-- which experiment CI runs:
--   target  (default) - /GL on this target only, CLNG built without /GL
--                       (the current, measured configuration)
--   clng              - CLNG (the static library dependency) built with /GL,
--                       this target WITHOUT /GL: matches the observation that
--                       the original's CLNG internals are LTCG-merged while the
--                       author's InputSink still calls a standalone
--                       FocusRecovery function (no cross-TU inlining)
--   project           - project-wide policy: CLNG and this target both /GL
--   off               - no LTO at all
-- PRISM_NO_LTO=1 is kept as an alias for off. The policy MUST be set before
-- includes() so it reaches CLNG's target; setting it earlier is also what
-- exposes the LNK2001 __std_regex_transform_primary_char question (see
-- build.yml: the earlier failure was the auto-downloaded prebuilt release lib,
-- which is now disabled by dropping CLNG's .git, so a source-built /GL CLNG may
-- link cleanly - that is exactly the measurement this knob exists to take).
local lto_scope = os.getenv("PRISM_LTO_SCOPE") or "target"
if os.getenv("PRISM_NO_LTO") == "1" then
    lto_scope = "off"
end
if is_mode("release") and (lto_scope == "clng" or lto_scope == "project") then
    set_policy("build.optimization.lto", true)
end

includes("lib/CommonLibSSE-NG/xmake.lua")

-- Project
set_project(PROJECT_NAME)
set_version(PROJECT_VERSION)
set_languages("cxx23")
set_license("unknown")

set_runtimes("MD")
set_allowedplats("windows")
set_allowedarchs("x64")
set_defaultplat("windows")
set_defaultarchs("x64")

add_rules("mode.debug", "mode.release")

if is_mode("debug") then
    add_defines("DEBUG")
    set_optimize("none")
elseif is_mode("release") then
    add_defines("NDEBUG")
    -- /O2, not /Ox: the original was built by CommonLibSSE-NG's CMake Release
    -- configuration, which leaves CMAKE_CXX_FLAGS_RELEASE at the MSVC default
    -- (/O2 /Ob2). xmake's "fastest" maps to /Ox, which schedules differently.
    set_optimize("faster")
    set_symbols("debug")
end

target(PROJECT_NAME)
    set_kind("shared")
    set_basename(PROJECT_NAME)

    -- Whole-program optimisation. LTCG is part of the configuration being
    -- reproduced (Rich header Utc1920_LTCG_CPP/_C; CLNG builds with IPO on
    -- Release), but its SCOPE is the open question - see the PRISM_LTO_SCOPE
    -- block before includes(). `target` (default) compiles this target /GL and
    -- keeps CLNG plain: that was the only configuration CI could link, because
    -- the project-scope policy used to pull the auto-downloaded PREBUILT CLNG
    -- release library (a foreign toolset) into the merge and the 14.44 link-time
    -- code generator then failed on LNK2001 __std_regex_transform_primary_char.
    -- The parity job now drops CLNG's .git so it always compiles from source, and
    -- `clng`/`project` exist to measure whether a source-built /GL CLNG links and
    -- matches; `off` (also PRISM_NO_LTO=1) disables LTO entirely.
    if is_mode("release") and lto_scope == "target" then
        set_policy("build.optimization.lto", true)
    elseif is_mode("release") and lto_scope == "clng" then
        -- The project-scope policy set before includes() also reaches this
        -- target; turn it back off here so only CLNG is merged. The link still
        -- gets /LTCG because CLNG's objects carry /GL.
        set_policy("build.optimization.lto", false)
    end
    -- CommonLibSSE-NG. We deliberately do NOT use the `commonlibsse-ng.plugin`
    -- rule: it injects a generated SKSEPlugin_Version/SKSEPlugin_Query from
    -- PluginDeclaration, and this reconstruction must emit the original's own
    -- version-INDEPENDENT PluginVersionData export instead.
    add_deps("commonlibsse-ng")

    set_pcxxheader("src/PCH.h")
    add_files("src/**.cpp")
    -- The original carries a VERSIONINFO resource (.rsrc, 752 bytes, no
    -- OriginalFilename/CompanyName). src/Version.rc reproduces it byte-for-byte
    -- as far as the resource contents go.
    add_files("src/Version.rc")
    add_headerfiles("src/**.h")
    add_includedirs("src")

    -- MSVC flags, following SexLabpp.
    add_cxxflags(
        "cl::/cgthreads4",
        "cl::/diagnostics:caret",
        "cl::/external:W0",
        "cl::/fp:contract",
        "cl::/fp:except-",
        "cl::/guard:cf-",
        "cl::/permissive-",
        "cl::/Gy",
        "cl::/Zc:enumTypes",
        "cl::/Zc:preprocessor",
        "cl::/Zc:templateScope",
        "cl::/utf-8"
    )
    add_cxxflags("cl::/we4715") -- `function` : not all control paths return a value
    add_cxxflags(
        "cl::/wd4068", -- unknown pragma
        "cl::/wd4201", -- nonstandard extension used : nameless struct/union
        "cl::/wd4265"  -- class with virtual functions but non-virtual destructor
    )
    if is_mode("debug") then
        add_cxxflags("cl::/bigobj")
    elseif is_mode("release") then
        -- /Ob2 (not /Ob3) and /Gy plus whole-program optimisation: the original
        -- shows the Utc1920_LTCG_CPP/_C marker in its Rich header and its CLNG
        -- CMake sets CMAKE_INTERPROCEDURAL_OPTIMIZATION for Release, and its 59
        -- byte-identical function pools can only come from /Gy + /OPT:ICF.
        add_cxxflags("cl::/Zc:inline", "cl::/JMC-", "cl::/Ob2")
        -- /GL comes from the build.optimization.lto policy above.
        add_ldflags("/OPT:REF", "/OPT:ICF")
    end
target_end()