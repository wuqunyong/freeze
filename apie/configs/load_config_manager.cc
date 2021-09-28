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
		auto bResult = this->loadFile(elem.first);
		if (!bResult)
		{
			return bResult;
		}
	}

	return true;
}

}  // namespace blocker
