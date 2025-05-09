#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <concepts>
#include <unordered_map>

#include "apie/module_loader/module_loader.h"
#include "apie/event/timer_impl.h"
#include "apie/api/hook.h"

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

	template <typename T>
	bool hasModule();

	status::Status hookHandler(hook::HookPoint point);

	void checkStartFinish();
	void checkLoadFinish();

	bool checkExitFinish();

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
		ss << "registerModule | name: " << name << " | type:" << typeid(T).name() << "| module name collision";

		PANIC_ABORT(ss.str().c_str());
	}

	auto ptrCreate = this->getOrCreateLoader<T>(name, iPriority);
	if (ptrCreate == nullptr)
	{
		std::stringstream ss;                                                                                                                                  \
		ss << "registerModule | name: " << name << " | type:" << typeid(T).name() << "| not match";

		PANIC_ABORT(ss.str().c_str());
	}

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
		ss << "getModulePtr | name: " << name << " | type:" << typeid(T).name() << "| getLoader null";
		throw std::invalid_argument(ss.str());
	}

	auto ptrMudule = ptrLoader->getModulePtr();
	if (ptrMudule == nullptr)
	{
		std::stringstream ss;
		ss << "getModulePtr | name: " << name << " | type:" << typeid(T).name() << "| ptrMudule null";
		throw std::invalid_argument(ss.str());
	}

	return ptrMudule;
}

template <typename T>
bool ModuleLoaderManager::hasModule()
{
	std::string name = T::moduleName();

	auto ite = m_loader.find(name);
	if (ite != m_loader.end())
	{
		return true;
	}

	return false;
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
concept ModuleT = requires(T c, std::string sName, apie::module_loader::ModuleLoaderBase* prtLoader, apie::hook::HookPoint point) {

	{T::moduleName()} -> std::convertible_to<std::string>;
	{T::modulePrecedence()} -> std::convertible_to<uint32_t>;

	{ new T(sName, prtLoader) };

	c.setHookReady(point);
};

template <typename T>
requires ModuleT<T>
static inline bool RegisterModule()
{
	return apie::module_loader::ModuleLoaderMgrSingleton::get().registerModule<T>();
}

template <typename T>
requires ModuleT<T>
static inline bool HasModule()
{
	return apie::module_loader::ModuleLoaderMgrSingleton::get().hasModule<T>();
}

template <typename T>
requires ModuleT<T>
static inline auto GetModule()
{
	return apie::module_loader::ModuleLoaderMgrSingleton::get().getModulePtr<T>();
}
