#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <tuple>
#include <mutex>
#include <optional>

#include "apie/singleton/threadsafe_singleton.h"
#include "apie/status/status.h"

namespace apie {
namespace hook {


enum class HookPoint
{
	HP_Init = 0,
	HP_Start = 1,
	HP_Ready = 2,
	HP_Exit = 3,
};


class HookRegistry {
public:
	using HookCallback = std::function<apie::status::Status(HookPoint)>;

	friend void APieModuleObj(HookRegistry::HookCallback cb);

	struct HookEntry
	{
		HookCallback cb;
		uint32_t priority = 0;
	};

	using HookCallbackMap = std::map<HookPoint, std::vector<HookEntry>>;

	std::optional<std::vector<HookEntry>> getHook(HookPoint point);

	void triggerHook(HookPoint point);

private:
	//register
	void registerHook(HookPoint point, HookCallback cb, uint32_t priority = 0);

	HookCallbackMap m_hookMap;
	std::mutex m_hookMutex;
};

typedef ThreadSafeSingleton<HookRegistry> HookRegistrySingleton;


extern void APieModuleObj(HookRegistry::HookCallback cb);

} 
} 
