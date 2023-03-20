#include "apie/api/hook.h"

#include <chrono>
#include <string>
#include <iosfwd>
#include <sstream>
#include <algorithm> 

#include "apie/network/logger.h"
#include "apie/common/enum_to_int.h"

namespace apie {
namespace hook {

	void HookRegistry::registerHook(HookPoint point, HookCallback cb, uint32_t priority)
	{
		HookEntry entry;
		entry.cb = cb;
		entry.priority = priority;

		auto findIte = m_hookMap.find(point);
		if (findIte == m_hookMap.end())
		{
			std::vector<HookEntry> cbVec;
			cbVec.push_back(entry);
			m_hookMap[point] = cbVec;
		}
		else
		{
			findIte->second.push_back(entry);
		}
	}


	std::optional<std::vector<HookRegistry::HookEntry>> HookRegistry::getHook(HookPoint point)
	{
		auto findIte = m_hookMap.find(point);
		if (findIte == m_hookMap.end())
		{
			return std::nullopt;
		}

		return std::make_optional(findIte->second);
	}

	void HookRegistry::triggerHook(HookPoint point)
	{
		auto initCbOpt = apie::hook::HookRegistrySingleton::get().getHook(point);
		if (!initCbOpt.has_value())
		{
			return;
		}

		auto ptrCmp = [](HookEntry& lhs, HookEntry& rhs) {
			return lhs.priority < rhs.priority;
		};
		std::sort(initCbOpt.value().begin(), initCbOpt.value().end(), ptrCmp);

		bool bAllOk = true;
		for (auto& item : initCbOpt.value())
		{
			auto result = item.cb(point);
			if (!result.ok())
			{
				bAllOk = false;

				if ((point == HookPoint::HP_Start || point == HookPoint::HP_Load)
					&& result.isAsync())
				{
					continue;
				}

				std::stringstream ss;
				ss << "errorCode:" << apie::toUnderlyingType(result.code()) << "|info:" << result.message();

				if (point == HookPoint::HP_Exit)
				{
					PIE_FMT_LOG(PIE_NOTICE, "startup/startup|exit|{}", ss.str().c_str());
					continue;
				}

				PANIC_ABORT(ss.str().c_str());
			}
		}

		if (bAllOk && point == HookPoint::HP_Start)
		{
			apie::hook::HookRegistrySingleton::get().triggerHook(hook::HookPoint::HP_Load);
		}

		if (bAllOk && point == HookPoint::HP_Load)
		{
			apie::hook::HookRegistrySingleton::get().triggerHook(hook::HookPoint::HP_Ready);
		}
	}

	void APieModuleObj(HookRegistry::HookCallback cb)
	{
		apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Init, cb);
		apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Start, cb);
		apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Load, cb);
		apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Ready, cb);
		apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Exit, cb);
	}

} // namespace Hook
} // namespace APie
