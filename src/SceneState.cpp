#include "SceneState.h"

#include "PCH.h"
#include "FocusRecovery.h"
#include "Json.h"
#include "UiBridge.h"

#include <atomic>
#include <mutex>
#include <utility>

namespace SceneState
{
	namespace
	{
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

		// fmt literal at 0x18005d6a0, field order preserved exactly.
		std::string json;
		json.reserve(256 + a_actorNames.size() * 64);
		json += "{\"active\":";
		json += a_active ? "true" : "false";
		json += ",\"thread\":";
		json += std::to_string(a_threadID);
		json += ",\"status\":";
		json += std::to_string(a_status);
		json += ",\"sceneId\":";
		PrismJson::AppendQuoted(json, a_sceneID);
		json += ",\"sceneName\":";
		PrismJson::AppendQuoted(json, a_sceneName);
		json += ",\"stage\":";
		PrismJson::AppendQuoted(json, a_stage);
		json += ",\"stageIndex\":";
		json += std::to_string(a_stageIdx);
		json += ",\"stageCount\":";
		json += std::to_string(a_stageCount);
		json += ",\"freecam\":";
		json += a_freecam ? "true" : "false";
		json += ",\"paused\":";
		json += a_paused ? "true" : "false";
		json += ",\"muted\":";
		json += a_muted ? "true" : "false";
		json += ",\"speed\":";
		PrismJson::AppendFixed2(json, a_speed);
		json += ",\"actors\":[";
		for (std::size_t i = 0; i < a_actorNames.size(); ++i) {
			if (i != 0) {
				json += ',';
			}
			json += "{\"name\":";
			PrismJson::AppendQuoted(json, a_actorNames[i]);
			json += ",\"enjoyment\":";
			json += std::to_string(i < a_enjoyment.size() ? a_enjoyment[i] : 0);
			json += ",\"player\":";
			json += (static_cast<std::int32_t>(i) == a_playerIdx) ? "true" : "false";
			json += '}';
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
		std::string json;
		json.reserve(a_sceneIDs.size() * 24 + 2);
		json += '[';
		for (std::size_t i = 0; i < a_sceneIDs.size(); ++i) {
			if (i != 0) {
				json += ',';
			}
			PrismJson::AppendQuoted(json, a_sceneIDs[i]);
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