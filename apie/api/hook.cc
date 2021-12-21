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
		if (initCbOpt.has_value())
		{
			auto ptrCmp = [](HookEntry& lhs, HookEntry& rhs){
				return lhs.priority < rhs.priority;
			};
			std::sort(initCbOpt.value().begin(), initCbOpt.value().end(), ptrCmp);

			for (auto& item : initCbOpt.value())
			{
				auto result = item.cb();
				if (!result.ok())
				{
					std::stringstream ss;
					ss << "errorCode:" << apie::toUnderlyingType(result.errorCode()) << "|info:" << result.errorMessage();

					if (point == HookPoint::HP_Exit)
					{
						PIE_LOG("startup/startup", PIE_CYCLE_DAY, PIE_NOTICE, "exit|%s", ss.str().c_str());
						continue;
					}

					PANIC_ABORT(ss.str().c_str());
				}
			}
		}
	}

	void APieModuleInit(HookRegistry::HookCallback cb)
	{
		apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Init, cb);
	}

	void APieModuleStart(HookRegistry::HookCallback cb)
	{
		apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Start, cb);
	}

	void APieModuleReady(HookRegistry::HookCallback cb)
	{
		apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Ready, cb);
	}

	void APieModuleExit(HookRegistry::HookCallback cb)
	{
		apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Exit, cb);
	}

} // namespace Hook
} // namespace APie
