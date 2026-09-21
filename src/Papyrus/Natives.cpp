#include "Papyrus/Natives.h"

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

	// --- catalogue state (DAT_1800951f0) -----------------------------------
	std::mutex                            g_catalogMutex;
	std::vector<Catalog::Record>          g_catalogRecords;
	std::unordered_map<std::string, std::size_t, Catalog::Fnv1aHash> g_catalogIndex;
	std::int32_t                          g_catalogExpectedTotal = 0;
	std::atomic<bool>                     g_catalogReady         = false;
	bool                                  g_catalogBuilding      = false;
	std::uint64_t                         g_catalogBuildStart    = 0;

	std::uint64_t NowTicks()
	{
		// Original: FUN_18001d7d0 (a QPC-family clock) stored at DAT_18009c238
		// and diffed in nanoseconds (recon/NATIVES-RECOVERED.md §3.9).
		return static_cast<std::uint64_t>(
			std::chrono::steady_clock::now().time_since_epoch().count());
	}

	// The Papyrus side hands tags as one joined string (papyrusUtil.StringJoin).
	// The UI consumes an array, so split here. GUESS: the original's split
	// delimiter was not recovered; accept ',', ';' and '|'.
	std::vector<std::string> SplitTags(std::string_view a_tags)
	{
		std::vector<std::string> out;
		std::string              current;
		for (const char c : a_tags) {
			if (c == ',' || c == ';' || c == '|') {
				if (!current.empty()) {
					out.push_back(std::move(current));
					current.clear();
				}
			} else {
				current += c;
			}
		}
		if (!current.empty()) {
			out.push_back(std::move(current));
		}
		return out;
	}

	std::string RowsJson(const std::vector<Catalog::Record>& a_records)
	{
		std::string json;
		json.reserve(a_records.size() * 96 + 2);
		json += '[';
		bool first = true;
		for (const auto& record : a_records) {
			if (!first) {
				json += ',';
			}
			first = false;
			json += "{\"id\":";
			PrismJson::AppendQuoted(json, record.id);
			json += ",\"name\":";
			PrismJson::AppendQuoted(json, record.name);
			json += ",\"tags\":[";
			bool firstTag = true;
			for (const auto& tag : record.tags) {
				if (!firstTag) {
					json += ',';
				}
				firstTag = false;
				PrismJson::AppendQuoted(json, tag);
			}
			json += "],\"package\":";
			PrismJson::AppendQuoted(json, record.package);
			json += '}';
		}
		json += ']';
		return json;
	}

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

	// 0x18002a000 — clear/reserve the 128-byte-record vector, reset the id index.
	void Papyrus_CatalogBegin(RE::StaticFunctionTag*, std::int32_t a_total)
	{
		{
			std::lock_guard lock{ g_catalogMutex };
			g_catalogRecords.clear();
			g_catalogIndex.clear();
			if (a_total > 0) {
				g_catalogRecords.reserve(static_cast<std::size_t>(a_total));
			}
			g_catalogExpectedTotal = a_total;
			g_catalogBuildStart    = NowTicks();
		}
		g_catalogReady    = false;
		g_catalogBuilding = true;
#line 480 "src\\main.cpp"
		logger::info("Catalog build started: {} registered scenes expected", a_total);
#line 112
		UiBridge::InvokeJs("slppCatalogReset", std::to_string(a_total));
	}

	// 0x180029740 — append records, refresh the FNV-1a id -> index map.
	void Papyrus_CatalogAppend(
		RE::StaticFunctionTag*,
		std::vector<std::string> a_ids,
		std::vector<std::string> a_names,
		std::vector<std::string> a_tags)
	{
		std::string json;
		json += '[';
		bool first = true;
		{
			std::lock_guard lock{ g_catalogMutex };
			for (std::size_t i = 0; i < a_ids.size(); ++i) {
				const auto& id = a_ids[i];
				if (id.empty()) {
					continue;
				}
				Catalog::Record record;
				record.id   = id;
				record.name = (i < a_names.size() && !a_names[i].empty()) ? a_names[i] : id;
				if (i < a_tags.size()) {
					record.tags = SplitTags(a_tags[i]);
				}
				const auto index = g_catalogRecords.size();
				g_catalogRecords.push_back(std::move(record));
				g_catalogIndex[g_catalogRecords.back().id] = index;

				const auto& row = g_catalogRecords.back();
				if (!first) {
					json += ',';
				}
				first = false;
				json += "{\"id\":";
				PrismJson::AppendQuoted(json, row.id);
				json += ",\"name\":";
				PrismJson::AppendQuoted(json, row.name);
				json += ",\"tags\":[";
				bool firstTag = true;
				for (const auto& tag : row.tags) {
					if (!firstTag) {
						json += ',';
					}
					firstTag = false;
					PrismJson::AppendQuoted(json, tag);
				}
				json += "],\"package\":";
				PrismJson::AppendQuoted(json, row.package);
				json += '}';
			}
		}
		json += ']';
		if (!first) {
			UiBridge::PushCatalogChunk(json,
				static_cast<std::int32_t>(g_catalogRecords.size()), g_catalogExpectedTotal);
		}
	}

	// 0x18002a410 — assign a package name to already-appended records.
	void Papyrus_CatalogPackage(RE::StaticFunctionTag*, std::string a_package, std::vector<std::string> a_ids)
	{
		std::lock_guard lock{ g_catalogMutex };
		std::size_t     mapped = 0;
		for (const auto& id : a_ids) {
			const auto it = g_catalogIndex.find(id);
			if (it != g_catalogIndex.end() && it->second < g_catalogRecords.size()) {
				g_catalogRecords[it->second].package = a_package;
				++mapped;
			}
		}
#line 533 "src\\main.cpp"
		logger::info("Catalog package '{}' mapped to {}/{} scenes", a_package, mapped, a_ids.size());
#line 161
	}

	// 0x18002a270 — ready/building flags + elapsed-ms log + UI push.
	void Papyrus_CatalogFinish(RE::StaticFunctionTag*)
	{
		std::size_t count = 0;
		{
			std::lock_guard lock{ g_catalogMutex };
			count = g_catalogRecords.size();
		}
		g_catalogReady    = true;
		g_catalogBuilding = false;
		const std::uint64_t ms = (NowTicks() - g_catalogBuildStart) / 1'000'000ull;
#line 547 "src\\main.cpp"
		logger::info("Catalog build finished: {} scenes in {} ms", count, ms);
#line 150
		UiBridge::InvokeJs("slppCatalogDone", std::to_string(count));
	}

	// 0x18002a8a0 — 9-byte leaf: movzx byte [ready].
	bool Papyrus_IsCatalogReady(RE::StaticFunctionTag*)
	{
		return g_catalogReady.load();
	}

	// 0x18002a830 — (end - begin) >> 7.
	std::int32_t Papyrus_GetCatalogCount(RE::StaticFunctionTag*)
	{
		std::lock_guard lock{ g_catalogMutex };
		return static_cast<std::int32_t>(g_catalogRecords.size());
	}

	// 0x18002a6e0 — hand the cached catalogue to the UI.
	void Papyrus_CatalogPublish(RE::StaticFunctionTag*)
	{
		std::vector<Catalog::Record> snapshot;
		{
			std::lock_guard lock{ g_catalogMutex };
			snapshot = g_catalogRecords;
		}
#line 577 "src\\main.cpp"
		logger::info("Catalog session cache reused: {} scenes already in UI", snapshot.size());
#line 177
		UiBridge::PushCatalog(RowsJson(snapshot), static_cast<std::int32_t>(snapshot.size()));
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
namespace Catalog
{
	std::size_t Fnv1aHash::operator()(std::string_view a_key) const noexcept
	{
		std::uint64_t hash = 0xcbf29ce484222325ull;
		for (const char c : a_key) {
			hash ^= static_cast<unsigned char>(c);
			hash *= 0x100000001b3ull;
		}
		return static_cast<std::size_t>(hash);
	}

	void RetryPublish(std::int32_t a_attempt)
	{
		std::vector<Record> snapshot;
		{
			std::lock_guard lock{ g_catalogMutex };
			snapshot = g_catalogRecords;
		}

		constexpr std::int32_t kMaxRetries = 5;
		if (a_attempt >= kMaxRetries) {
			logger::warn("Catalog still incomplete after {} retries", a_attempt);
			return;
		}

		const std::size_t chunkSize = (static_cast<std::size_t>(64) >> a_attempt) > 1
			? (static_cast<std::size_t>(64) >> a_attempt)
			: 1;
		logger::info("Catalog incomplete on UI side, republishing with smaller slices (attempt {})", a_attempt);

		const auto total = static_cast<std::int32_t>(snapshot.size());
		UiBridge::InvokeJs("slppCatalogReset", std::to_string(total));
		for (std::size_t i = 0; i < snapshot.size(); i += chunkSize) {
			const std::size_t remaining = snapshot.size() - i;
			const std::size_t end       = i + (chunkSize < remaining ? chunkSize : remaining);
			std::vector<Record> chunk(snapshot.begin() + static_cast<std::ptrdiff_t>(i),
			snapshot.begin() + static_cast<std::ptrdiff_t>(end));
			UiBridge::InvokeJs("slppCatalogChunk", RowsJson(chunk));
		}
		UiBridge::InvokeJs("slppCatalogDone", std::to_string(total));
	}
}  // namespace Catalog

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
		a_vm->RegisterFunction("CatalogBegin"sv, kClassName, Papyrus_CatalogBegin, true);
		a_vm->RegisterFunction("CatalogAppend"sv, kClassName, Papyrus_CatalogAppend, true);
		a_vm->RegisterFunction("CatalogPackage"sv, kClassName, Papyrus_CatalogPackage, true);
		a_vm->RegisterFunction("CatalogFinish"sv, kClassName, Papyrus_CatalogFinish, true);
		a_vm->RegisterFunction("IsCatalogReady"sv, kClassName, Papyrus_IsCatalogReady, true);
		a_vm->RegisterFunction("GetCatalogCount"sv, kClassName, Papyrus_GetCatalogCount, true);
		a_vm->RegisterFunction("CatalogPublish"sv, kClassName, Papyrus_CatalogPublish, true);
		a_vm->RegisterFunction("SetSearchQuery"sv, kClassName, Papyrus_SetSearchQuery, true);

		logger::info("Registered {} natives on {}", kNativeNames.size(), kClassName);
		return true;
	}
}  // namespace Papyrus::Natives
