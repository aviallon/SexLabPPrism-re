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

-- CommonLibSSE-NG (alandtse/CommonLibVR `ng`). Must exist before this script is
-- parsed; the workflow checks it out first, and tools/check_commonlib.py proves
-- it is a format-5-capable revision.
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
    set_optimize("fastest")
    set_symbols("debug")
end

target(PROJECT_NAME)
    set_kind("shared")
    set_basename(PROJECT_NAME)

    -- CommonLibSSE-NG. We deliberately do NOT use the `commonlibsse-ng.plugin`
    -- rule: it injects a generated SKSEPlugin_Version/SKSEPlugin_Query from
    -- PluginDeclaration, and this reconstruction must emit the original's own
    -- version-INDEPENDENT PluginVersionData export instead.
    add_deps("commonlibsse-ng")

    set_pcxxheader("src/PCH.h")
    add_files("src/**.cpp")
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
        add_cxxflags("cl::/Zc:inline", "cl::/JMC-", "cl::/Ob3")
    end
target_end()