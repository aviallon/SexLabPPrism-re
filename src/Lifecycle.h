#pragma once

// Lifecycle — the SKSE messaging listener (`OnMessage`).
//
// Recovered from 0x180029200 (recon/decompiled/0x180029200_OnMessage.c):
// SKSEPlugin_Load registers this on the "SKSE" messaging channel; when the
// data-loaded message arrives (the original compares against the literal 8,
// `MessagingInterface::kDataLoaded` in current CLNG), it builds the PrismaUI
// view/callbacks and registers the two event sinks.
namespace Lifecycle
{
	// Register the "SKSE" messaging listener. Called from SKSEPlugin_Load.
	void Register();
}  // namespace Lifecycle