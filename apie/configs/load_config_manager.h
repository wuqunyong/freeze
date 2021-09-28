#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "apie/singleton/threadsafe_singleton.h"
#include "apie/configs/load_config.h"

namespace apie {

class LoadConfigManager {
 public:
    using ConfigMap = std::unordered_map<std::string, std::shared_ptr<LoadConfigBase>>;

    LoadConfigManager();
	virtual ~LoadConfigManager();


	template <typename T>
	bool registerConfig(const std::string& name, const std::string& fileName);

	template <typename T>
	std::shared_ptr<LoadConfig<T>> getConfig(const std::string& name);

    bool loadAll();
    bool reloadFile(const std::string& fileName);

private:
	bool loadFile(const std::string& fileName);

 private:
    ConfigMap configs_;
    std::unordered_map<std::string, std::string> file_name_map_;
};


template <typename T>
bool LoadConfigManager::registerConfig(const std::string& name, const std::string& fileName)
{
    if (configs_.find(name) != configs_.end())
    {
        return false;
    }

	if (file_name_map_.find(fileName) != file_name_map_.end())
	{
		return false;
	}

    auto configObj = std::make_shared<LoadConfig<T>>(name);
    configs_[name] = configObj;

    file_name_map_[fileName] = name;

    return true;
}


template <typename T>
std::shared_ptr<LoadConfig<T>> LoadConfigManager::getConfig(const std::string& name)
{
    std::shared_ptr<LoadConfig<T>> ptrLoadConf = nullptr;

    auto findIte = configs_.find(name);
    if (findIte != configs_.end()) 
    {
        ptrLoadConf = std::dynamic_pointer_cast<LoadConfig<T>>(findIte->second);
    }
    return ptrLoadConf;
}


using LCMgrSingleton = apie::ThreadSafeSingleton<LoadConfigManager>;

}  // namespace apie
