#include "SceneState.h"

#include "PCH.h"
#include "FocusRecovery.h"
#include "Json.h"
#include "UiBridge.h"

#include <atomic>
#include <format>
#include <mutex>
#include <string_view>
#include <utility>

// The original's per-string pre-quoting helper (FUN_18002bee0) returns a
// std::string that already carries the surrounding quotes and the JSON
// escapes. Inside std::format it travels as a user-defined type (MSVC's
// "custom" format arg, type 0xc) whose formatter writes the pre-quoted text
// verbatim. Declaring the type at file scope lets us specialise
// std::formatter<JsonQuoted>; keeping the escaper out-of-line reproduces the
// original's real `call FUN_18002bee0` at every string field.
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

namespace SceneState
{
	namespace
	{
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
		// DAT_18009c1cb is touched under LOCK()/UNLOCK() in the original, including
		// from SetSearchQuery which does not hold the session mutex.
		std::atomic<bool> g_modalSearchOpen = false;

		// --- published blobs (DAT_180095130) -----------------------------------
		std::mutex  g_jsonMutex;
		std::string g_stateJson;       // DAT_18009c190
		std::string g_compatibleJson;  // DAT_180095180

		// FocusRecovery (0x180013290) is the free-camera reassertion state machine
		// recovered in recon/BINARY-RECON.md §1.4. The natives only ever call its
		// Cancel() entry point.
	}  // namespace

	std::int32_t BeginSceneSession()
	{
		std::lock_guard lock{ g_sessionMutex };
		const auto      id = ++g_sessionCounter;
#line 372
		logger::info("Scene session {} allocated", id);
#line 44
		return id;
	}

	bool IsFreeCameraActive()
	{
		// The original resolves the PlayerCamera singleton through two RELOCATION_IDs
		// (FUN_180039120) and then compares [cam+0x28] == [cam+0xd0] (FUN_180039180):
		// free camera engaged iff the current camera state equals the free state.
		// CommonLibSSE-NG exposes exactly that as PlayerCamera::GetSingleton() +
		// PlayerCamera::IsInFreeCameraMode() (include/RE/P/PlayerCamera.h:135/142),
		// so use the wrapper rather than hand-resolving the address-library ids.
		const auto* const camera = RE::PlayerCamera::GetSingleton();
		return camera != nullptr && camera->IsInFreeCameraMode();
	}

	void Publish(
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
			// Pinned to the original's source_loc line (immediate 0x18a=394)
			// recovered from FUN_18002ae90 at recon/decompiled/0x18002ae90_Papyrus_PublishSceneState.c.
#line 394
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
			// Pinned source_loc line 0x195=405 (same FUN_18002ae90).
#line 405
			logger::warn("Rejected invalid scene activation: session {} closed={} thread={} status={}",
				a_session, g_wasActiveLatch, a_threadID, a_status);
#line 91
			return;
		}

		// fmt literal at 0x18005d6a0 (std::format, spdlog std_format=true), field
		// order preserved exactly. The three strings are pre-quoted first, then
		// formatted once through std::vformat with 12 args, exactly as the
		// original calls its format helper.
		constexpr std::string_view kSceneFormat =
			"{{\"active\":{},\"thread\":{},\"status\":{},\"sceneId\":{},\"sceneName\":{},\"stage\":{},\"stageIndex\":{},\"stageCount\":{},\"freecam\":{},\"paused\":{},\"muted\":{},\"speed\":{:.2f},\"actors\":[";
		constexpr std::string_view kActorFormat =
			"{{\"name\":{},\"enjoyment\":{},\"player\":{}}}";

		const std::string qSceneID   = QuoteJson(a_sceneID);
		const std::string qSceneName = QuoteJson(a_sceneName);
		const std::string qStage     = QuoteJson(a_stage);
		// MSVC's make_format_args only accepts lvalues.
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
				activeStr,
				a_threadID,
				a_status,
				argSceneID,
				argSceneName,
				argStage,
				a_stageIdx,
				a_stageCount,
				freecamStr,
				pausedStr,
				mutedStr,
				a_speed));

		for (std::size_t i = 0; i < a_actorNames.size(); ++i) {
			if (i != 0) {
				json += ',';
			}
			const std::string  qName = QuoteJson(a_actorNames[i]);
			std::int32_t enjoyment = i < a_enjoyment.size() ? a_enjoyment[i] : 0;
			const char* playerStr = (static_cast<std::int32_t>(i) == a_playerIdx) ? "true" : "false";
			JsonQuoted   argName{ qName };
			json += std::vformat(
				kActorFormat,
				std::make_format_args(argName, enjoyment, playerStr));
		}
		json += "]}";

		const bool wasActive = g_sceneActive;
		if (!a_active || a_status != 3) {
			g_sceneActive = false;
			if (wasActive) {
				FocusRecovery::Cancel();
				g_modalSearchOpen.store(false);
				// Pinned source_loc line 0x1b8=440 (same FUN_18002ae90).
#line 440
				logger::info("Scene state: became INACTIVE (thread {}, status {})", a_threadID, a_status);
#line 126
			}
		} else {
			g_sceneActive = true;
			if (!wasActive) {
				FocusRecovery::Cancel();
				g_uiMode          = true;
				g_interfaceHidden = false;
				// Pinned source_loc line 0x1b4=436 (same FUN_18002ae90).
#line 436
				logger::info("Scene state: became ACTIVE (thread {}, scene '{}')", a_threadID, a_sceneID);
#line 136
			}
		}

		{
			std::lock_guard jsonLock{ g_jsonMutex };
			g_stateJson = json;
		}
		lock.unlock();

		UiBridge::PushState(json);
	}

	void PublishCompatible(const std::vector<std::string>& a_sceneIDs)
	{
		// The original does not use std::format here: it builds "[" then appends
		// a real call to the pre-quoting helper (FUN_18002bee0) per scene id,
		// separated by ',', then ']' (recon/decompiled/0x18002a9a0).
		std::string json;
		json += '[';
		for (std::size_t i = 0; i < a_sceneIDs.size(); ++i) {
			if (i != 0) {
				json += ',';
			}
			json += QuoteJson(a_sceneIDs[i]);
		}
		json += ']';

		// Pinned to the original's source_loc line (immediate 0x1cc=460)
		// recovered from FUN_18002a9a0 at recon/decompiled/0x18002a9a0_Papyrus_PublishCompatible.c.
#line 460
		logger::info("Compatible scene list published: {} scenes", a_sceneIDs.size());
#line 154
		{
			std::lock_guard jsonLock{ g_jsonMutex };
			g_compatibleJson = json;
		}
		UiBridge::PushCompatible(json);
	}

	void SetSearchQuery(std::string a_query)
	{
		g_modalSearchOpen.store(false);
#line 586
		logger::info("Modal search completed: {} characters", a_query.size());
#line 190
		UiBridge::SetSearchQuery(a_query);
	}

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
}  // namespace SceneState