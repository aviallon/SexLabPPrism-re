#include "Papyrus/Natives.h"

namespace Papyrus::Natives
{
	namespace
	{
		constexpr std::string_view kClassName = "SexLabPrismNative"sv;

		// STEP 4 scope: the plugin must compile, load and expose the exact native
		// surface. Behaviour of the Prism state/catalog natives is not recoverable
		// from the binary (recon/BINARY-RECON.md §5), so every one of them logs a
		// single line and returns a safe default. `Log` is the one native whose
		// behaviour is fully determined by the contract, so it forwards to spdlog.
		void NotImplemented(std::string_view a_name) { logger::warn("{}: not implemented yet", a_name); }
	}  // namespace

	void Log(RE::StaticFunctionTag*, std::string a_message)
	{
		logger::info("[Papyrus] {}", a_message);
	}

	std::int32_t BeginSceneSession(RE::StaticFunctionTag*)
	{
		NotImplemented("BeginSceneSession"sv);
		return 0;  // no active scene session
	}

	void PublishSceneState(
		RE::StaticFunctionTag*,
		std::int32_t,
		bool,
		std::int32_t,
		std::int32_t,
		std::string,
		std::string,
		std::string,
		std::int32_t,
		std::int32_t,
		bool,
		bool,
		bool,
		float,
		std::vector<RE::BSFixedString>,
		std::vector<std::int32_t>,
		std::int32_t)
	{
		NotImplemented("PublishSceneState"sv);
	}

	bool IsFreeCameraActive(RE::StaticFunctionTag*)
	{
		NotImplemented("IsFreeCameraActive"sv);
		return false;  // free camera not engaged
	}

	void PublishCompatible(RE::StaticFunctionTag*, std::vector<RE::BSFixedString>)
	{
		NotImplemented("PublishCompatible"sv);
	}

	void CatalogBegin(RE::StaticFunctionTag*, std::int32_t)
	{
		NotImplemented("CatalogBegin"sv);
	}

	void CatalogAppend(
		RE::StaticFunctionTag*,
		std::vector<RE::BSFixedString>,
		std::vector<RE::BSFixedString>,
		std::vector<RE::BSFixedString>)
	{
		NotImplemented("CatalogAppend"sv);
	}

	void CatalogPackage(RE::StaticFunctionTag*, std::string, std::vector<RE::BSFixedString>)
	{
		NotImplemented("CatalogPackage"sv);
	}

	void CatalogFinish(RE::StaticFunctionTag*)
	{
		NotImplemented("CatalogFinish"sv);
	}

	bool IsCatalogReady(RE::StaticFunctionTag*)
	{
		NotImplemented("IsCatalogReady"sv);
		return false;  // catalogue never reports ready
	}

	std::int32_t GetCatalogCount(RE::StaticFunctionTag*)
	{
		NotImplemented("GetCatalogCount"sv);
		return 0;  // empty catalogue
	}

	void CatalogPublish(RE::StaticFunctionTag*)
	{
		NotImplemented("CatalogPublish"sv);
	}

	void SetSearchQuery(RE::StaticFunctionTag*, std::string)
	{
		NotImplemented("SetSearchQuery"sv);
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