#pragma once

// CommonLibSSE-NG (alandtse/CommonLibVR, branch `ng`). The build script pulls
// the LATEST `ng` HEAD and asserts include/REL/IDDB.h has the format-5 reader
// (Format::SSEv5) before compiling; see tools/check_commonlib.py.
#pragma warning(push)
#pragma warning(disable : 4200)
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#pragma warning(pop)

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace logger = SKSE::log;
using namespace std::literals;