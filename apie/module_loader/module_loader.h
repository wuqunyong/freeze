#pragma once

#include <stddef.h>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "apie/api/hook.h"
#include "apie/common/macros.h"
#include "apie/network/logger.h"

namespace apie {
namespace module_loader {

DEFINE_TYPE_TRAIT(HasInit, init)
DEFINE_TYPE_TRAIT(HasStart, start)
DEFINE_TYPE_TRAIT(HasReady, ready)
DEFINE_TYPE_TRAIT(HasExit, exit)


class ModuleLoaderBase {
public:
	virtual ~ModuleLoaderBase() = default;

	virtual std::string name() = 0;
	virtual uint32_t getPriority() = 0;
	virtual apie::status::Status init() = 0;
	virtual apie::status::Status start() = 0;
	virtual apie::status::Status ready() = 0;
	virtual apie::status::Status exit() = 0;

	bool getHookReady(hook::HookPoint point)
	{
		auto ite = m_hookReady.find(point);
		if (ite == m_hookReady.end())
		{
			return false;
		}

		return ite->second;
	}

	void setHookReady(hook::HookPoint point)
	{
		m_hookReady[point] = true;
	}

private:
	std::map<hook::HookPoint, bool> m_hookReady;
};


template <typename T>
class ModuleLoader : public ModuleLoaderBase {

public:
	using ModuleType = T;
	using ModulePtr = std::shared_ptr<T>;

	explicit ModuleLoader(const std::string& name, uint32_t priority);
	virtual ~ModuleLoader();

	std::string name() override;
	uint32_t getPriority() override;

	apie::status::Status init() override;
	apie::status::Status start() override;
	apie::status::Status ready() override;
	apie::status::Status exit() override;

	ModulePtr getModulePtr();

private:
	std::string m_name;
	uint32_t m_priority;
	ModulePtr m_modulePtr;
};

template <typename T>
ModuleLoader<T>::ModuleLoader(const std::string& name, uint32_t priority) 
	: m_name(name),
	m_priority(priority)
{
	m_modulePtr = std::make_shared<T>(name, this);
}

template <typename T>
ModuleLoader<T>::~ModuleLoader()
{

}

template <typename T>
std::string ModuleLoader<T>::name()
{
	return m_name;
}

template <typename T>
uint32_t ModuleLoader<T>::getPriority()
{
	return m_priority;
}

template <typename T>
auto ModuleLoader<T>::getModulePtr() -> ModulePtr {
	return m_modulePtr;
}

template <typename T>
apie::status::Status ModuleLoader<T>::init()
{
	if constexpr (HasInit<T>::value)
	{
		return m_modulePtr->init();
	}
	else
	{
		return status::Status(status::StatusCode::OK, "");
	}
}

template <typename T>
apie::status::Status ModuleLoader<T>::start()
{
	if constexpr (HasInit<T>::value)
	{
		return m_modulePtr->start();
	}
	else
	{
		return status::Status(status::StatusCode::OK, "");
	}
}

template <typename T>
apie::status::Status ModuleLoader<T>::ready()
{
	if constexpr (HasReady<T>::value)
	{
		return m_modulePtr->ready();
	}
	else
	{
		return status::Status(status::StatusCode::OK, "");
	}
}

template <typename T>
apie::status::Status ModuleLoader<T>::exit()
{
	if constexpr (HasExit<T>::value)
	{
		return m_modulePtr->exit();
	}
	else
	{
		return status::Status(status::StatusCode::OK, "");
	}
}


} 
}