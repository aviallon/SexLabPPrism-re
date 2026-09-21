#pragma once

#include "Papyrus/Natives.h"

// The catalogue natives live in their own TU (src/Papyrus/CatalogNatives.cpp)
// so the CatalogAppend body work can proceed without colliding with the
// session/state natives in src/Papyrus/Natives.cpp.
//
// The bodies themselves are free functions named Papyrus_<Name> in the GLOBAL
// anonymous namespace of CatalogNatives.cpp, exactly like the original's
// `anonymous-namespace'::Papyrus_CatalogAppend. Because an anonymous namespace
// has internal linkage, the registration table cannot name them from another
// TU, so this TU exposes one registration entry point instead.
namespace Papyrus::Natives
{
	// Register the 7 catalogue natives on `SexLabPrismNative`.
	void RegisterCatalog(VM* a_vm);
}  // namespace Papyrus::Natives
