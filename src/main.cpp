#include "Papyrus/Natives.h"
#include "Lifecycle.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// SexLabPPrism 0.6.1 — SKSE plugin entry points.
//
// The three exports mirror the original SexLabPPrism.dll (recon/BINARY-RECON.md §4):
//   SKSEPlugin_Query    infoVersion=1, name="SexLabPPrism", version=0x00060010
//   SKSEPlugin_Version  version-independent PluginVersionData (dataVersion=1)
//   SKSEPlugin_Load     CommonLibSSE-NG init + Papyrus native registration
namespace
{
	constexpr std::string_view kPluginName          = "SexLabPPrism"sv;
	constexpr std::uint32_t    kPluginVersionPacked = 0x00060010;  // 0.6.1.0

	// 0x180027d20 SetupLog(): the original does NOT use CLNG's `log::init`
	// (SKSE::Init is called with a_log=false). It builds its own logger named
	// "SexLabPPrism" writing SexLabPPrism.log through a basic_file_sink plus a
	// Windows colour stdout sink, with pattern "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
	// (recon/strings.txt:217). The absence of CLNG's msvc_sink/LogEventHandler
	// and of the "[%T.%e] [%=5t]" pattern in the original is the evidence.
	void SetupLog()
	{
		auto path = SKSE::log::log_directory();
		if (!path) {
			return;
		}
		*path /= "SexLabPPrism.log";

		std::vector<spdlog::sink_ptr> sinks{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true),
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
		};
		auto pluginLogger = std::make_shared<spdlog::logger>("SexLabPPrism", sinks.begin(), sinks.end());
		pluginLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
		spdlog::set_default_logger(std::move(pluginLogger));
	}
}  // namespace

// Export 1: the version-independent plugin description, emitted as DATA exactly
// like the original. The original 0.6.1 struct is
//   +0x00 dataVersion   = 1
//   +0x04 pluginVersion = 0x00060010
//   +0x08 pluginName    = "SexLabPPrism"
// with everything else (author, email, independence flags, compatibleVersions)
// zeroed, i.e. no compatible-runtime table at all.
SKSE_EXPORT constinit SKSE::PluginVersionData SKSEPlugin_Version = []() constexpr noexcept {
	SKSE::PluginVersionData data{};
	data.PluginVersion(REL::Version{ 0, 6, 1, 0 });
	data.PluginName(kPluginName);
	// Zero these explicitly: PluginVersionData's default member initializer would
	// otherwise claim AddressLibraryV5 support, which the original does not set.
	data.versionIndependenceEx = 0;
	data.versionIndependence   = 0;
	return data;
}();

// Export 2: the classic query. No game-version check (the original has none);
// the plugin relies on the Address Library and fails at Init if it cannot load.
SKSE_EXPORT bool SKSEPlugin_Query(SKSE::QueryInterface*, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name        = kPluginName.data();
	a_info->version     = kPluginVersionPacked;
	return true;
}

// Export 3: load. SetupLog() builds the project logger (file + stdout), then
// SKSE::Init initialises the address library and interfaces; the version banner
// and the native registration follow.
#include "missing/MissingGroup1.h"
#include "missing/MissingGroup2.h"
#include "missing/MissingGroup3.h"
#include "missing/MissingRound2Group1.h"
#include "missing/MissingRound2Group2.h"
#include "missing/MissingRound2Group3.h"
#include "missing/MissingRound3.h"
#include "missing/MissingRound4.h"

SKSE_EXPORT bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	// 0x18002e25d: SetupLog() runs BEFORE SKSE::Init, and a_log is false so
	// CLNG's own log::init()/msvc_sink is never instantiated.
	SetupLog();
	SKSE::Init(a_skse, false);

	// Matching-decomp force-links, NOT behaviour: the parity build compiles with
	// /Gy and links with /OPT:REF, so any function nothing references is stripped
	// from the DLL and becomes invisible to the comparison. These three calls keep
	// the reconstructed bodies in src/missing/MissingGroup*.cpp alive. Each group
	// XORs a volatile sink over its whole table, because observing only one element
	// lets the optimiser fold the table and the linker strip every other body.
	ForceLink_MissingGroup1();
	ForceLink_MissingGroup2();
	ForceLink_MissingGroup3();
	ForceLink_Missing4Group1();
	ForceLink_Missing5Group2();
	ForceLink_MissingRound2Group3();
	ForceLink_Missing7();
	ForceLink_MissingRound4();

	logger::info("SexLab P+ Prism 0.6.1 loading (strict lifecycle + confirmed freecam + selective UI)");

	// 0x18002e2da / 0x18002e2e2: both interface singletons are fetched before
	// either is checked; a null for EITHER returns false (0x18002e2ea/ef).
	const auto papyrus   = SKSE::GetPapyrusInterface();
	const auto messaging = SKSE::GetMessagingInterface();
	if (!papyrus || !messaging) {
		logger::critical("Failed to get the SKSE interfaces");
		return false;
	}

	// 0x18002e2fe: Register_Impl(papyrus, 0x180026a80)
	if (!papyrus->Register(Papyrus::Natives::Register)) {
		logger::critical("Failed to register the SexLabPrismNative natives");
		return false;
	}

	// 0x18002e314: RegisterListener("SKSE", OnMessage). The original inlines the
	// listener registration here; Lifecycle::Register keeps the same call.
	Lifecycle::Register();

	// 0x18002e319: return true. The original logs no completion banner, so the
	// reconstruction does not either.
	return true;
}