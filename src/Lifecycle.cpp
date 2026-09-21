#include "Lifecycle.h"

#include "PCH.h"
#include "InputSink.h"
#include "MenuVisibilitySink.h"
#include "PrismaUI.h"

// The original's listener body is the GLOBAL anonymous-namespace
// ``anonymous-namespace'::OnMessage`` (src\main.cpp, unity build). It is placed
// at file scope here (not inside `namespace Lifecycle`) so the compiler bakes
// that exact __FUNCSIG__ -- the identity anchor.
namespace
{
	// The original's literal comparison: `*(int *)(msg + 8) != 8 -> return`.
	// In the current CLNG MessagingInterface enum, 8 is kDataLoaded; the
	// literal is kept so the load point matches the original exactly.
	constexpr std::uint32_t kPrismMessageType = 8;

	// The original's linker split the `PrismaUI API not found` error path of
	// OnMessage out of the hot body (a separate .pdata function at 0x1800295a4,
	// 23 instructions, sharing the parent epilogue). Keeping it in its own
	// __declspec(noinline) body is the closest source-level counterpart: it
	// stays a real function the matcher can anchor instead of being folded into
	// OnMessage and vanishing. The parameter is the OnMessage message so the
	// recovered signature keeps the same `struct SKSE::MessagingInterface::
	// Message *` argument shape.
	__declspec(noinline)
	void OnMessageColdBlock(SKSE::MessagingInterface::Message*)
	{
		logger::warn("PrismaUI API not found");
	}

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (!a_msg || a_msg->type != kPrismMessageType) {
			return;
		}

		// CreateViews() loads PrismaUI.dll by name, RequestPluginAPI(1),
		// creates "SexLabPPrism/controller-0.6.1.html" and registers the six
		// JS callbacks (recon/NATIVES-RECOVERED.md §4.1). The sinks are only
		// registered on the success path, matching the original.
		if (!PrismaUI::CreateViews()) {
			OnMessageColdBlock(a_msg);
			return;
		}

		InputSink::Register();
		MenuVisibilitySink::Register();

		logger::info("SexLab P+ Prism ready");
	}
}  // namespace

namespace Lifecycle
{
	void Register()
	{
		const auto messaging = SKSE::GetMessagingInterface();
		if (!messaging) {
			logger::critical("Failed to get the messaging interface");
			return;
		}
		if (!messaging->RegisterListener("SKSE", OnMessage)) {
			logger::critical("Failed to register messaging listener for {}", "SKSE");
		}
	}
}  // namespace Lifecycle