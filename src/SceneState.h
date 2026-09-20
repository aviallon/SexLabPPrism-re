#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// Scene lifecycle state shared by the scene natives.
//
// Mirrors the original globals (recon/NATIVES-RECOVERED.md §0/§3.2–3.5):
//   session counter, latest-published session id, was-active latch,
//   sceneActive, uiMode, interfaceHidden, modalSearchOpen, plus the two
//   JSON blobs (scene state + compatible list) behind their own mutex.
namespace SceneState
{
	// DAT_18009c1d0, guarded by the session mutex.
	std::int32_t BeginSceneSession();

	// PublishSceneState. Returns after either rejecting (stale / invalid
	// activation) or building the state JSON and queueing a UI push.
	void Publish(
		std::int32_t               a_session,
		bool                       a_active,
		std::int32_t               a_threadID,
		std::int32_t               a_status,
		std::string                a_sceneID,
		std::string                a_sceneName,
		std::string                a_stage,
		std::int32_t               a_stageIdx,
		std::int32_t               a_stageCount,
		bool                       a_freecam,
		bool                       a_paused,
		bool                       a_muted,
		float                      a_speed,
		std::vector<std::string>   a_actorNames,
		std::vector<std::int32_t>  a_enjoyment,
		std::int32_t               a_playerIdx);

	bool IsFreeCameraActive();

	void PublishCompatible(const std::vector<std::string>& a_sceneIDs);

	void SetSearchQuery(std::string a_query);

	// Used by the (not yet reconstructed) InputSink/MenuVisibilitySink.
	[[nodiscard]] bool IsSceneActive();
}  // namespace SceneState