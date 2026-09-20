#include "UiBridge.h"

#include "PCH.h"
#include "PrismaUI.h"

#include <functional>
#include <string>

namespace UiBridge
{
	namespace
	{
		template <class F>
		void QueueOnGameThread(F&& a_callable)
		{
			auto* const task = SKSE::GetTaskInterface();
			if (!task) {
				logger::warn("SKSE TaskInterface unavailable; dropping Prism UI update");
				return;
			}
			task->AddTask(std::function<void()>(std::forward<F>(a_callable)));
		}
	}  // namespace

	bool IsAvailable()
	{
		return PrismaUI::IsAvailable();
	}

	void InvokeJs(const char* a_functionName, std::string_view a_argument)
	{
		PrismaUI::InvokeJs(a_functionName, a_argument);
	}

	void PushState(std::string_view a_json)
	{
		const std::string json{ a_json };
		QueueOnGameThread([json]() { PrismaUI::InvokeJs("slppState", json); });
	}

	void PushCompatible(std::string_view a_json)
	{
		const std::string json{ a_json };
		QueueOnGameThread([json]() { PrismaUI::InvokeJs("slppSetCompatible", json); });
	}

	void SetSearchQuery(std::string_view a_query)
	{
		const std::string query{ a_query };
		QueueOnGameThread([query]() { PrismaUI::InvokeJs("slppSetSearchQuery", query); });
	}

	void PushCatalog(std::string_view a_rowsJson, std::int32_t a_total)
	{
		const std::string rows{ a_rowsJson };
		const auto        total = std::to_string(a_total);
		QueueOnGameThread([rows, total]() {
			PrismaUI::InvokeJs("slppCatalogReset", total);
			PrismaUI::InvokeJs("slppCatalogChunk", rows);
			PrismaUI::InvokeJs("slppCatalogDone", total);
		});
	}

	void PushCatalogChunk(std::string_view a_rowsJson, std::int32_t a_loaded, std::int32_t a_total)
	{
		const std::string rows{ a_rowsJson };
		const auto        loaded = std::to_string(a_loaded);
		(void)a_total;  // slppCatalogProgress(loaded, total) takes two JS args;
		// the recovered InteropCall is single-argument (see PrismaUI.h), so only
		// `loaded` is sent and the JS `total || catalogTotal` fallback keeps the
		// total from slppCatalogReset. Marked: two-arg interop was not recovered.
		QueueOnGameThread([rows, loaded]() {
			PrismaUI::InvokeJs("slppCatalogChunk", rows);
			PrismaUI::InvokeJs("slppCatalogProgress", loaded);
		});
	}
}  // namespace UiBridge