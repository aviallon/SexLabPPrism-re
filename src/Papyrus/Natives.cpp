#include "Papyrus/Natives.h"

#include "Catalog.h"
#include "SceneState.h"

namespace Papyrus::Natives
{
	namespace
	{
		constexpr std::string_view kClassName = "SexLabPrismNative"sv;

		std::string ToStr(const RE::BSFixedString& a_value)
		{
			return std::string(static_cast<std::string_view>(a_value));
		}

		std::vector<std::string> ToStrVector(const std::vector<RE::BSFixedString>& a_values)
		{
			std::vector<std::string> out;
			out.reserve(a_values.size());
			for (const auto& value : a_values) {
				out.emplace_back(ToStr(value));
			}
			return out;
		}
	}  // namespace

	// 0x18002a8e0 — spdlog only, '[Papyrus] {}'.
	void Log(RE::StaticFunctionTag*, std::string a_message)
	{
		logger::info("[Papyrus] {}", a_message);
	}

	// 0x180029630 — mutex + monotonic session counter.
	std::int32_t BeginSceneSession(RE::StaticFunctionTag*)
	{
		return SceneState::BeginSceneSession();
	}

	// 0x18002ae90 — stale rejection, latch/thread/status validation, state JSON,
	// active/inactive transition handling, UI push through the task interface.
	void PublishSceneState(
		RE::StaticFunctionTag*,
		std::int32_t                   aiSession,
		bool                           abActive,
		std::int32_t                   aiThreadID,
		std::int32_t                   aiStatus,
		std::string                    asSceneID,
		std::string                    asSceneName,
		std::string                    asStage,
		std::int32_t                   aiStageIdx,
		std::int32_t                   aiStageCount,
		bool                           abFreecam,
		bool                           abPaused,
		bool                           abMuted,
		float                          afSpeed,
		std::vector<RE::BSFixedString> asActorNames,
		std::vector<std::int32_t>      aiEnjoyment,
		std::int32_t                   aiPlayerIdx)
	{
		SceneState::Publish(
			aiSession, abActive, aiThreadID, aiStatus,
			std::move(asSceneID), std::move(asSceneName), std::move(asStage),
			aiStageIdx, aiStageCount, abFreecam, abPaused, abMuted, afSpeed,
			ToStrVector(asActorNames), std::move(aiEnjoyment), aiPlayerIdx);
	}

	// 0x18002a8b0 — PlayerCamera free-camera predicate.
	bool IsFreeCameraActive(RE::StaticFunctionTag*)
	{
		return SceneState::IsFreeCameraActive();
	}

	// 0x18002a9a0 — JSON array of scene ids + UI push.
	void PublishCompatible(RE::StaticFunctionTag*, std::vector<RE::BSFixedString> asSceneIDs)
	{
		SceneState::PublishCompatible(ToStrVector(asSceneIDs));
	}

	// 0x18002a000 — clear/reserve the 128-byte-record vector, reset the id index.
	void CatalogBegin(RE::StaticFunctionTag*, std::int32_t aiTotal)
	{
		Catalog::Begin(aiTotal);
	}

	// 0x180029740 — append records, refresh the FNV-1a id -> index map.
	void CatalogAppend(
		RE::StaticFunctionTag*,
		std::vector<RE::BSFixedString> asIDs,
		std::vector<RE::BSFixedString> asNames,
		std::vector<RE::BSFixedString> asTags)
	{
		Catalog::Append(ToStrVector(asIDs), ToStrVector(asNames), ToStrVector(asTags));
	}

	// 0x18002a410 — assign a package name to already-appended records.
	void CatalogPackage(RE::StaticFunctionTag*, std::string asPackage, std::vector<RE::BSFixedString> asIDs)
	{
		Catalog::Package(asPackage, ToStrVector(asIDs));
	}

	// 0x18002a270 — ready/building flags + elapsed-ms log + UI push.
	void CatalogFinish(RE::StaticFunctionTag*)
	{
		Catalog::Finish();
	}

	// 0x18002a8a0 — 9-byte leaf: movzx byte [ready].
	bool IsCatalogReady(RE::StaticFunctionTag*)
	{
		return Catalog::IsReady();
	}

	// 0x18002a830 — (end - begin) >> 7.
	std::int32_t GetCatalogCount(RE::StaticFunctionTag*)
	{
		return Catalog::Count();
	}

	// 0x18002a6e0 — hand the cached catalogue to the UI.
	void CatalogPublish(RE::StaticFunctionTag*)
	{
		Catalog::Publish();
	}

	// 0x18002bd20 — clear the modal flag and forward the query to the UI.
	void SetSearchQuery(RE::StaticFunctionTag*, std::string asQuery)
	{
		SceneState::SetSearchQuery(std::move(asQuery));
	}

	bool Register(VM* a_vm)
	{
		if (!a_vm) {
			logger::critical("Papyrus: null virtual machine, cannot register SexLabPrismNative");
			return false;
		}

		// Exactly the 13 natives from recon/PAPYRUS-CONTRACT.md §1. The 4th
		// argument marks the function callable from tasklets, matching the
		// original "global native" declarations.
		a_vm->RegisterFunction("Log"sv, kClassName, Log, true);
		a_vm->RegisterFunction("BeginSceneSession"sv, kClassName, BeginSceneSession, true);
		a_vm->RegisterFunction("PublishSceneState"sv, kClassName, PublishSceneState, true);
		a_vm->RegisterFunction("IsFreeCameraActive"sv, kClassName, IsFreeCameraActive, true);
		a_vm->RegisterFunction("PublishCompatible"sv, kClassName, PublishCompatible, true);
		a_vm->RegisterFunction("CatalogBegin"sv, kClassName, CatalogBegin, true);
		a_vm->RegisterFunction("CatalogAppend"sv, kClassName, CatalogAppend, true);
		a_vm->RegisterFunction("CatalogPackage"sv, kClassName, CatalogPackage, true);
		a_vm->RegisterFunction("CatalogFinish"sv, kClassName, CatalogFinish, true);
		a_vm->RegisterFunction("IsCatalogReady"sv, kClassName, IsCatalogReady, true);
		a_vm->RegisterFunction("GetCatalogCount"sv, kClassName, GetCatalogCount, true);
		a_vm->RegisterFunction("CatalogPublish"sv, kClassName, CatalogPublish, true);
		a_vm->RegisterFunction("SetSearchQuery"sv, kClassName, SetSearchQuery, true);

		logger::info("Registered {} natives on {}", kNativeNames.size(), kClassName);
		return true;
	}
}  // namespace Papyrus::Natives