#pragma once

#include "apie/configs/load_config_manager.h"
#include "apie/common/file.h"

namespace apie {


LoadConfigManager::LoadConfigManager()
{
}

LoadConfigManager::~LoadConfigManager()
{
}

bool LoadConfigManager::loadFile(const std::string& fileName)
{
	auto fileIte = file_name_map_.find(fileName);
	if (fileIte == file_name_map_.end())
	{
		return false;
	}

	auto configIte = configs_.find(fileIte->second);
	if (configIte == configs_.end())
	{
		return false;
	}

	std::string content;
	bool bResult = apie::common::GetContent(fileName, &content);
	if (!bResult)
	{
		return false;
	}

	return configIte->second->load(content);
}

bool LoadConfigManager::reloadFile(const std::string& fileName)
{
	auto fileIte = file_name_map_.find(fileName);
	if (fileIte == file_name_map_.end())
	{
		return false;
	}

	auto configIte = configs_.find(fileIte->second);
	if (configIte == configs_.end())
	{
		return false;
	}

	std::string content;
	bool bResult = apie::common::GetContent(fileName, &content);
	if (!bResult)
	{
		return false;
	}

	return configIte->second->reload(content);
}

bool LoadConfigManager::loadAll()
{
	for (const auto& elem : file_name_map_)
	{
		bool bResult = this->loadFile(elem.first);
		if (!bResult)
		{
			return bResult;
		}

		auto findIte = configs_.find(elem.second);
		if (findIte == configs_.end())
		{
			std::stringstream ss;
			ss << "configObj not exist|fileName:" << elem.second;                                                                                                                         \

			ASYNC_PIE_LOG("load_config", PIE_CYCLE_DAY, PIE_ERROR, "%s", ss.str().c_str());

			return false;
		}

		std::string errInfo;
		bResult = findIte->second->isValid(errInfo);
		if (!bResult)
		{
			std::stringstream ss;
			ss << "isValid error|fileName:" << elem.second << "|errInfo:" << errInfo;                                                                                                                         \

			ASYNC_PIE_LOG("load_config", PIE_CYCLE_DAY, PIE_ERROR, "%s", ss.str().c_str());

			return bResult;
		}
	}

	return true;
}

}  // namespace blocker
