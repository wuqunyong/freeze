#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "apie/module_loader/module_loader.h"
#include "apie/event/timer_impl.h"

namespace apie {
namespace module_loader {

class ModuleLoaderManager {
public:
	using LoaderMap = std::unordered_map<std::string, std::shared_ptr<ModuleLoaderBase>>;

	ModuleLoaderManager();
	virtual ~ModuleLoaderManager();

	template <typename T>
	bool registerModule();

	template <typename T>
	auto getModulePtr();

	status::Status hookHandler(hook::HookPoint point);
	void checkStartFinish();

	static apie::status::Status APieModuleHookHandler(hook::HookPoint point);

private:
	template <typename T>
	std::shared_ptr<ModuleLoader<T>> getLoader(const std::string& name);

	template <typename T>
	std::shared_ptr<ModuleLoader<T>> getOrCreateLoader(const std::string& name, uint32_t priority);

	bool m_init = false;
	LoaderMap m_loader;
	std::chrono::milliseconds m_timeOut;
	std::chrono::milliseconds m_stepDuration;
};


template <typename T>
bool ModuleLoaderManager::registerModule()
{
	std::string name = T::moduleName();
	uint32_t iPriority = T::modulePrecedence();

	auto ptrLoader = this->getLoader<T>(name);
	if (ptrLoader != nullptr)
	{
		std::stringstream ss;
		ss << "module name collision: " << name;

		PANIC_ABORT(ss.str().c_str());
		return false;
	}

	this->getOrCreateLoader<T>(name, iPriority);
	return true;
}

template <typename T>
auto ModuleLoaderManager::getModulePtr()
{
	std::string name = T::moduleName();
	auto ptrLoader = this->getLoader<T>(name);
	if (ptrLoader == nullptr)
	{
		std::stringstream ss;
		ss << "unregister module: " << name;
		throw std::invalid_argument(ss.str());
	}

	return ptrLoader->getModulePtr();
}


template <typename T>
std::shared_ptr<ModuleLoader<T>> ModuleLoaderManager::getLoader(const std::string& name)
{
	std::shared_ptr<ModuleLoader<T>> ptrLoader = nullptr;

	auto ite = m_loader.find(name);
	if (ite != m_loader.end()) 
	{
		ptrLoader = std::dynamic_pointer_cast<ModuleLoader<T>>(ite->second);
	}

	return ptrLoader;
}

template <typename T>
std::shared_ptr<ModuleLoader<T>> ModuleLoaderManager::getOrCreateLoader(const std::string& name, uint32_t priority)
{
	std::shared_ptr<ModuleLoader<T>> ptrLoader = nullptr;

	auto ite = m_loader.find(name);
	if (ite != m_loader.end())
	{
		ptrLoader = std::dynamic_pointer_cast<ModuleLoader<T>>(ite->second);
	}
	else 
	{
		ptrLoader = std::make_shared<ModuleLoader<T>>(name, priority);
		m_loader[name] = ptrLoader;
	}

	return ptrLoader;
}


using ModuleLoaderMgrSingleton = ThreadSafeSingleton<ModuleLoaderManager>;


}
}

template <typename T>
static inline bool APieRegisterModule()
{
	return apie::module_loader::ModuleLoaderMgrSingleton::get().registerModule<T>();
}

template <typename T>
static inline auto APieGetModule()
{
	return apie::module_loader::ModuleLoaderMgrSingleton::get().getModulePtr<T>();
}