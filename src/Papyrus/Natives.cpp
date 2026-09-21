#include "Papyrus/Natives.h"
#include "Papyrus/CatalogNatives.h"

#include "PCH.h"
#include "Catalog.h"
#include "FocusRecovery.h"
#include "Json.h"
#include "Presentation.h"
#include "PrismaUI.h"
#include "SceneState.h"
#include "UiBridge.h"

#include <atomic>
#include <chrono>
#include <format>
#include <mutex>
#include <unordered_map>
#include <utility>

// ---------------------------------------------------------------------------
// The original compiles every native *body* in `src\main.cpp` inside the file's
// GLOBAL anonymous namespace, so each body's baked __FUNCSIG__ reads
//   `anonymous-namespace'::Papyrus_<Name>(RE::StaticFunctionTag*, ...)
// (verified with `strings artifacts/SexLabPPrism.dll`). A body living in a
// class/namespace (SceneState::*, Catalog::*) can never produce that name, no
// matter how identical the logic is, and the spdlog source_loc at each log call
// also encodes the original's file + line. The bodies therefore live here.
// The `#[line N "src\\main.cpp"]` pins below set both the recorded filename and
// the line the source_loc bakes.
//
// Signatures are byte-for-byte the original's, recovered from the FUNCSIG
// strings (and the Papyrus_CatalogAppend lambda mangling):
//   Papyrus_CatalogAppend(RE::StaticFunctionTag*, vector<string>, vector<string>, vector<string>)
// ---------------------------------------------------------------------------

namespace
{
	struct JsonQuoted
	{
		std::string_view text;
	};
}  // namespace

template <>
struct std::formatter<JsonQuoted>
{
	constexpr auto parse(std::format_parse_context& a_ctx) { return a_ctx.begin(); }

	auto format(const JsonQuoted& a_value, std::format_context& a_ctx) const
	{
		return std::format_to(a_ctx.out(), "{}", a_value.text);
	}
};

namespace
{
	constexpr std::string_view kClassName = "SexLabPrismNative"sv;

	// FUN_18002bee0: "..." with JSON escaping. Out-of-line on purpose so the
	// natives issue the same real call the original does.
	std::string QuoteJson(std::string_view a_in)
	{
		std::string out;
		out.reserve(a_in.size() + 2);
		out += '"';
		for (const char c : a_in) {
			switch (c) {
			case '"':  out += "\\\""; break;
			case '\\': out += "\\\\"; break;
			case '\n': out += "\\n"; break;
			case '\r': out += "\\r"; break;
			case '\t': out += "\\t"; break;
			default:
				if (static_cast<unsigned char>(c) < 0x20) {
					char buf[8];
					std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
					out += buf;
				} else {
					out += c;
				}
			}
		}
		out += '"';
		return out;
	}

	// --- session state (DAT_1800951a0) -------------------------------------
	std::mutex   g_sessionMutex;
	std::int32_t g_sessionCounter = 0;   // DAT_18009c1d0
	std::int32_t g_latestSession  = 0;   // DAT_18009c1d4
	bool         g_wasActiveLatch = false;  // DAT_180095123
	bool         g_sceneActive    = false;  // DAT_18009c1c9
	bool         g_uiMode         = false;  // DAT_180095120
	bool         g_interfaceHidden = false;  // DAT_18009c1ca
	std::atomic<bool> g_modalSearchOpen = false;  // DAT_18009c1cb

	// --- published blobs (DAT_180095130) -----------------------------------
	std::mutex  g_jsonMutex;
	std::string g_stateJson;       // DAT_18009c190
	std::string g_compatibleJson;  // DAT_180095180

	// =======================================================================
	// Natives — exact original signatures, global anonymous namespace.
	// =======================================================================

	// 0x18002a8e0 — spdlog only, '[Papyrus] {}'.
	void Papyrus_Log(RE::StaticFunctionTag*, std::string a_message)
	{
#line 365 "src\\main.cpp"
		logger::info("[Papyrus] {}", a_message);
#line 33
	}

	// 0x180029630 — mutex + monotonic session counter.
	std::int32_t Papyrus_BeginSceneSession(RE::StaticFunctionTag*)
	{
		std::lock_guard lock{ g_sessionMutex };
		const auto      id = ++g_sessionCounter;
#line 372 "src\\main.cpp"
		logger::info("Scene session {} allocated", id);
#line 44
		return id;
	}

	// 0x18002a8b0 — PlayerCamera free-camera predicate.
	bool Papyrus_IsFreeCameraActive(RE::StaticFunctionTag*)
	{
		const auto* const camera = RE::PlayerCamera::GetSingleton();
		return camera != nullptr && camera->IsInFreeCameraMode();
	}

	// 0x18002ae90 — stale rejection, latch/thread/status validation, state JSON,
	// active/inactive transition handling, UI push through the task interface.
	void Papyrus_PublishSceneState(
		RE::StaticFunctionTag*,
		std::int32_t              a_session,
		bool                      a_active,
		std::int32_t              a_threadID,
		std::int32_t              a_status,
		std::string               a_sceneID,
		std::string               a_sceneName,
		std::string               a_stage,
		std::int32_t              a_stageIdx,
		std::int32_t              a_stageCount,
		bool                      a_freecam,
		bool                      a_paused,
		bool                      a_muted,
		float                     a_speed,
		std::vector<std::string>  a_actorNames,
		std::vector<std::int32_t> a_enjoyment,
		std::int32_t              a_playerIdx)
	{
		std::unique_lock lock{ g_sessionMutex };

		if (a_session < g_latestSession) {
#line 394 "src\\main.cpp"
			logger::warn("Rejected stale scene publication: session {} < current {} (active={}, thread={})",
				a_session, g_latestSession, a_active, a_threadID);
#line 80
			return;
		}
		if (g_latestSession < a_session) {
			g_wasActiveLatch = false;
			g_latestSession  = a_session;
		}
		if (!a_active) {
			g_wasActiveLatch = true;
		} else if (g_wasActiveLatch || a_threadID < 0 || a_status != 3) {
#line 405 "src\\main.cpp"
			logger::warn("Rejected invalid scene activation: session {} closed={} thread={} status={}",
				a_session, g_wasActiveLatch, a_threadID, a_status);
#line 91
			return;
		}

		constexpr std::string_view kSceneFormat =
			"{{\"active\":{},\"thread\":{},\"status\":{},\"sceneId\":{},\"sceneName\":{},\"stage\":{},\"stageIndex\":{},\"stageCount\":{},\"freecam\":{},\"paused\":{},\"muted\":{},\"speed\":{:.2f},\"actors\":[";
		constexpr std::string_view kActorFormat =
			"{{\"name\":{},\"enjoyment\":{},\"player\":{}}}";

		const std::string qSceneID   = QuoteJson(a_sceneID);
		const std::string qSceneName = QuoteJson(a_sceneName);
		const std::string qStage     = QuoteJson(a_stage);
		const char* activeStr  = a_active ? "true" : "false";
		const char* freecamStr = a_freecam ? "true" : "false";
		const char* pausedStr  = a_paused ? "true" : "false";
		const char* mutedStr   = a_muted ? "true" : "false";
		JsonQuoted  argSceneID{ qSceneID };
		JsonQuoted  argSceneName{ qSceneName };
		JsonQuoted  argStage{ qStage };

		std::string json = std::vformat(
			kSceneFormat,
			std::make_format_args(
				activeStr, a_threadID, a_status,
				argSceneID, argSceneName, argStage,
				a_stageIdx, a_stageCount,
				freecamStr, pausedStr, mutedStr, a_speed));

		for (std::size_t i = 0; i < a_actorNames.size(); ++i) {
			if (i != 0) {
				json += ',';
			}
			const std::string  qName = QuoteJson(a_actorNames[i]);
			std::int32_t enjoyment = i < a_enjoyment.size() ? a_enjoyment[i] : 0;
			const char* playerStr = (static_cast<std::int32_t>(i) == a_playerIdx) ? "true" : "false";
			JsonQuoted   argName{ qName };
			json += std::vformat(kActorFormat, std::make_format_args(argName, enjoyment, playerStr));
		}
		json += "]}";

		const bool wasActive = g_sceneActive;
		if (!a_active || a_status != 3) {
			g_sceneActive = false;
			if (wasActive) {
				FocusRecovery::Cancel();
				g_modalSearchOpen.store(false);
#line 440 "src\\main.cpp"
				logger::info("Scene state: became INACTIVE (thread {}, status {})", a_threadID, a_status);
#line 126
			}
		} else {
			g_sceneActive = true;
			if (!wasActive) {
				FocusRecovery::Cancel();
				g_uiMode          = true;
				g_interfaceHidden = false;
#line 436 "src\\main.cpp"
				logger::info("Scene state: became ACTIVE (thread {}, scene '{}')", a_threadID, a_sceneID);
#line 136
			}
		}

		{
			std::lock_guard jsonLock{ g_jsonMutex };
			g_stateJson = json;
		}
		lock.unlock();

		// The original inlines its own no-arg `anonymous-namespace'::PushState()
		// (task lambda `PushState(void)::lambda_1`, 0x1800250f0) here; the bridge
		// exposes that shape, re-reading the state-JSON global when the task runs.
		UiBridge::PushState();
	}

	// 0x18002a9a0 — JSON array of scene ids + UI push.
	void Papyrus_PublishCompatible(RE::StaticFunctionTag*, std::vector<std::string> a_sceneIDs)
	{
		std::string json;
		json += '[';
		for (std::size_t i = 0; i < a_sceneIDs.size(); ++i) {
			if (i != 0) {
				json += ',';
			}
			json += QuoteJson(a_sceneIDs[i]);
		}
		json += ']';

#line 460 "src\\main.cpp"
		logger::info("Compatible scene list published: {} scenes", a_sceneIDs.size());
#line 154
		{
			std::lock_guard jsonLock{ g_jsonMutex };
			g_compatibleJson = json;
		}
		// Same shape for the compatible list: `anonymous-namespace'::
		// PushCompatible(void)::lambda_1`, no captures, global re-read.
		UiBridge::PushCompatible();
	}

	// 0x18002bd20 — clear the modal flag and forward the query to the UI.
	//
	// The original queues its OWN TaskInterface lambda here — the instantiated
	// wrapper is
	//   std::_Func_impl_no_alloc<`void __cdecl `anonymous-namespace'::
	//       Papyrus_SetSearchQuery(RE::StaticFunctionTag*,std::string)'::`2'::
	//       <lambda_1>,void>::vftable
	// (recon/decompiled/0x18002bd20_Papyrus_SetSearchQuery.c:64). The lambda
	// moves the query string into its capture and runs the PrismaUI dispatch:
	// it formats `window.slppSetSearchQuery(<quoted>);` and hands it to the
	// bridge's single guarded entry point (InvokeOn, 0x180028190), then applies
	// the presentation (0x180024450 tail). Routing through a UiBridge:: helper
	// would name the wrapper after UiBridge instead, so the dispatch lives here.
	void Papyrus_SetSearchQuery(RE::StaticFunctionTag*, std::string a_query)
	{
		g_modalSearchOpen.store(false);
#line 586 "src\\main.cpp"
		logger::info("Modal search completed: {} characters", a_query.size());
#line 190
		auto* const task = SKSE::GetTaskInterface();
		if (!task) {
			return;
		}
		task->AddTask([query = std::move(a_query)]() {
			const std::string code =
				"window.slppSetSearchQuery(" + QuoteJson(query) + ");";
			PrismaUI::InvokeOn(code);
			Presentation::ApplyPresentation();
		});
	}
}  // namespace

// ---------------------------------------------------------------------------
// Cross-TU accessors. These keep the state shared with the event sinks and the
// controller-driven actions; namespace qualification is compile-time only, so
// the native bodies above still read the same storage.
// ---------------------------------------------------------------------------

namespace SceneState
{
	bool IsSceneActive()
	{
		std::lock_guard lock{ g_sessionMutex };
		return g_sceneActive;
	}

	bool IsUiMode()
	{
		std::lock_guard lock{ g_sessionMutex };
		return g_uiMode;
	}

	void SetUiMode(bool a_uiMode)
	{
		std::lock_guard lock{ g_sessionMutex };
		g_uiMode = a_uiMode;
	}

	bool IsInterfaceHidden()
	{
		std::lock_guard lock{ g_sessionMutex };
		return g_interfaceHidden;
	}

	bool ToggleInterfaceHidden()
	{
		std::lock_guard lock{ g_sessionMutex };
		g_interfaceHidden = !g_interfaceHidden;
		return g_interfaceHidden;
	}

	bool IsModalSearchOpen()
	{
		return g_modalSearchOpen.load();
	}

	void SetModalSearchOpen(bool a_open)
	{
		g_modalSearchOpen.store(a_open);
	}

	std::string CurrentStateJson()
	{
		std::lock_guard lock{ g_jsonMutex };
		return g_stateJson;
	}

	// Read by UiBridge::PushCompatible()'s task lambda (060.1's
	// `PushCompatible(void)::lambda_1`), the twin of CurrentStateJson.
	std::string CurrentCompatibleJson()
	{
		std::lock_guard lock{ g_jsonMutex };
		return g_compatibleJson;
	}
}  // namespace SceneState

namespace Papyrus::Natives
{
	bool Register(VM* a_vm)
	{
		if (!a_vm) {
			logger::critical("Papyrus: null virtual machine, cannot register SexLabPrismNative");
			return false;
		}

		a_vm->RegisterFunction("Log"sv, kClassName, Papyrus_Log, true);
		a_vm->RegisterFunction("BeginSceneSession"sv, kClassName, Papyrus_BeginSceneSession, true);
		a_vm->RegisterFunction("PublishSceneState"sv, kClassName, Papyrus_PublishSceneState, true);
		a_vm->RegisterFunction("IsFreeCameraActive"sv, kClassName, Papyrus_IsFreeCameraActive, true);
		a_vm->RegisterFunction("PublishCompatible"sv, kClassName, Papyrus_PublishCompatible, true);
		RegisterCatalog(a_vm);  // 7 catalogue natives live in CatalogNatives.cpp
		a_vm->RegisterFunction("SetSearchQuery"sv, kClassName, Papyrus_SetSearchQuery, true);

		logger::info("Registered {} natives on {}", kNativeNames.size(), kClassName);
		return true;
	}
}  // namespace Papyrus::Natives
