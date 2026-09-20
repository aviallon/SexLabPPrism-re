#include "Papyrus/Natives.h"

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

// Export 3: load. SKSE::Init sets up the default logger, which writes
// Documents/My Games/Skyrim Special Edition/SKSE/SexLabPPrism.log (the filename
// comes from PluginVersionData::pluginName), then we log the version banner and
// register the 13 natives.
SKSE_EXPORT bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	logger::info("SexLab P+ Prism 0.6.1 loading (strict lifecycle + confirmed freecam + selective UI)");

	const auto papyrus = SKSE::GetPapyrusInterface();
	if (!papyrus) {
		logger::critical("Failed to get the Papyrus interface");
		return false;
	}
	if (!papyrus->Register(Papyrus::Natives::Register)) {
		logger::critical("Failed to register the SexLabPrismNative natives");
		return false;
	}

	logger::info("SexLab P+ Prism 0.6.1 initialisation complete");
	return true;
}