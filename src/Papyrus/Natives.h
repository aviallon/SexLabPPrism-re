#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// Papyrus binding for the `SexLabPrismNative` script.
//
// Recovered from artifacts/papyrus/SexLabPrismNative.pex (see
// recon/PAPYRUS-CONTRACT.md §1): one hidden script object named
// `SexLabPrismNative` with exactly 13 `global native` functions. These are the
// names the DLL registers; the argument lists mirror the PEX function
// signatures.
namespace Papyrus::Natives
{
	using VM = RE::BSScript::IVirtualMachine;

	// Register every native on `SexLabPrismNative`. Called from SKSEPlugin_Load.
	bool Register(VM* a_vm);

	// The exact 13 native names, in the order they appear in the original DLL
	// string table (recon/PAPYRUS-CONTRACT.md §1). Kept as a table so the count
	// and the names can be asserted at compile time and in verification.
	inline constexpr std::array<std::string_view, 13> kNativeNames{
		"Log",
		"BeginSceneSession",
		"PublishSceneState",
		"IsFreeCameraActive",
		"PublishCompatible",
		"CatalogBegin",
		"CatalogAppend",
		"CatalogPackage",
		"CatalogFinish",
		"IsCatalogReady",
		"GetCatalogCount",
		"CatalogPublish",
		"SetSearchQuery",
	};
	static_assert(kNativeNames.size() == 13, "SexLabPrismNative must register exactly 13 natives");

	// --- native implementations -------------------------------------------------
	void Log(RE::StaticFunctionTag*, std::string a_message);

	std::int32_t BeginSceneSession(RE::StaticFunctionTag*);

	void PublishSceneState(
		RE::StaticFunctionTag*,
		std::int32_t aiSession,
		bool         abActive,
		std::int32_t aiThreadID,
		std::int32_t aiStatus,
		std::string  asSceneID,
		std::string  asSceneName,
		std::string  asStage,
		std::int32_t aiStageIdx,
		std::int32_t aiStageCount,
		bool         abFreecam,
		bool         abPaused,
		bool         abMuted,
		float        afSpeed,
		std::vector<std::string>       asActorNames,
		std::vector<std::int32_t>      aiEnjoyment,
		std::int32_t                   aiPlayerIdx);

	bool IsFreeCameraActive(RE::StaticFunctionTag*);

	void PublishCompatible(RE::StaticFunctionTag*, std::vector<std::string> asSceneIDs);

	void CatalogBegin(RE::StaticFunctionTag*, std::int32_t aiTotal);

	void CatalogAppend(
		RE::StaticFunctionTag*,
		std::vector<std::string> asIDs,
		std::vector<std::string> asNames,
		std::vector<std::string> asTags);

	void CatalogPackage(
		RE::StaticFunctionTag*,
		std::string              asPackage,
		std::vector<std::string> asIDs);

	void CatalogFinish(RE::StaticFunctionTag*);

	bool IsCatalogReady(RE::StaticFunctionTag*);

	std::int32_t GetCatalogCount(RE::StaticFunctionTag*);

	void CatalogPublish(RE::StaticFunctionTag*);

	void SetSearchQuery(RE::StaticFunctionTag*, std::string asQuery);
}  // namespace Papyrus::Natives