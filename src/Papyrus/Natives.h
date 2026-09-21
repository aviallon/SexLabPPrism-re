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

	// The native bodies are free functions named Papyrus_<Name> in the GLOBAL
	// anonymous namespace of src/Papyrus/Natives.cpp (the original's shape).
	// Register() takes their addresses directly; no per-native declarations are
	// needed here.
}  // namespace Papyrus::Natives