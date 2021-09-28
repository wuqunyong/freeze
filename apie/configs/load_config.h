#pragma once

#include <stddef.h>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

#include <nlohmann/json.hpp>
#include "boost/pfr.hpp"

#include "apie/network/logger.h"

namespace apie {

class LoadConfigBase {
 public:
    virtual ~LoadConfigBase() = default;

    virtual std::string getConfigName() = 0;
    virtual bool load(const std::string& content) = 0;
    virtual bool reload(const std::string& content) = 0;
};


template <typename T>
class LoadConfig : public LoadConfigBase {
    friend class  LoadConfigManager;

 public:
    using ConfigType = T;
    using ConfigPtr = std::shared_ptr<T>;

    explicit LoadConfig(const std::string& name);
    virtual ~LoadConfig();

    std::string getConfigName() override;
    bool load(const std::string& content) override;
    bool reload(const std::string& content) override;

    const ConfigType& configData();

 private:
    std::string name_;
    ConfigType config_data_;
};

template <typename T>
LoadConfig<T>::LoadConfig(const std::string& name) 
    : name_(name)
{
}

template <typename T>
LoadConfig<T>::~LoadConfig() 
{
}

template <typename T>
std::string LoadConfig<T>::getConfigName()
{
    return name_;
}

template <typename T>
bool LoadConfig<T>::load(const std::string& content) 
{
    try
    {
		nlohmann::json jsonObj = nlohmann::json::parse(content);
		config_data_ = jsonObj.get<T>();
		return true;
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "load config error|fileName:" << name_ << "|exception:" << e.what();                                                                                                                         \
        
        ASYNC_PIE_LOG("load_config", PIE_CYCLE_DAY, PIE_ERROR, "%s", ss.str().c_str());

        return false;
    }
}

template <typename T>
bool LoadConfig<T>::reload(const std::string& content) 
{
    try
    {
		nlohmann::json jsonObj = nlohmann::json::parse(content);
		config_data_ = jsonObj.get<T>();
        return true;
    }
    catch (const std::exception& e)
    {
		std::stringstream ss;
		ss << "reload config error|fileName:" << name_ << "|exception:" << e.what();                                                                                                                         \

		ASYNC_PIE_LOG("load_config", PIE_CYCLE_DAY, PIE_ERROR, "%s", ss.str().c_str());

        return false;
    }
}

template <typename T>
const T& LoadConfig<T>::configData()
{
    return config_data_;
}


}  // namespace apie
