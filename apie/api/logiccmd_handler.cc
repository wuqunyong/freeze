#include "apie/api/logiccmd_handler.h"

#include <chrono>
#include <string>
#include <iosfwd>
#include <sstream>
#include <algorithm> 

#include "apie/network/logger.h"
#include "apie/common/file.h"


namespace apie {

	LogicCmdHandler::LogicCmdHandler()
	{
		if (!m_init)
		{
			this->registerOnCmd("help", "Display information about builtin commands", LogicCmdHandler::onHelp);
			this->registerOnCmd("reload", "Reload the configuration file", LogicCmdHandler::onReload);
			m_init = true;
		}
	}

	void LogicCmdHandler::init()
	{

	}

	bool LogicCmdHandler::registerOnCmd(const std::string& name, const std::string& desc, Callback cb)
	{
		auto findIte = m_cmd.find(name);
		if (findIte != m_cmd.end())
		{
			ASYNC_PIE_LOG("logic_cmd", PIE_CYCLE_DAY, PIE_ERROR, "duplicate|registerOnCmd:%s", name.c_str());
			return false;
		}

		cmd_entry_t entry;
		entry.name = name;
		entry.desc = desc;
		entry.cb = cb;

		m_cmd[entry.name] = entry;
		return true;
	}

	std::optional<LogicCmdHandler::Callback> LogicCmdHandler::findCb(const std::string& name)
	{
		auto findIte = m_cmd.find(name);
		if (findIte == m_cmd.end())
		{
			return std::nullopt;
		}

		return findIte->second.cb;
	}

	std::map<std::string, LogicCmdHandler::cmd_entry_t>& LogicCmdHandler::cmds()
	{
		return m_cmd;
	}

	void LogicCmdHandler::onHelp(::pubsub::LOGIC_CMD& cmd)
	{
		std::stringstream ss;

		ss << std::endl;
		for (const auto& items : LogicCmdHandlerSingleton::get().cmds())
		{
			auto iLen = items.second.name.size();
			ss << items.second.name;

			while (iLen < 32)
			{
				ss << " ";
				iLen++;
			}
			ss << "  ";
			ss << items.second.desc << std::endl;
		}

		ASYNC_PIE_LOG("logic_cmd", PIE_CYCLE_DAY, PIE_DEBUG, "%s", ss.str().c_str());
	}

	void LogicCmdHandler::onReload(::pubsub::LOGIC_CMD& cmd)
	{
		std::string configFile;
		try {
			configFile = apie::CtxSingleton::get().getConfigFile();

			int64_t mtime = apie::common::FileDataModificationTime(configFile);
			if (mtime == -1)
			{
				PIE_LOG("Exception/Exception", PIE_CYCLE_HOUR, PIE_ERROR, "reload|configFile:%s not exist", configFile.c_str());
				return;
			}

			if (apie::CtxSingleton::get().getConfigFileMTime() == mtime)
			{
				PIE_LOG("Exception/Exception", PIE_CYCLE_HOUR, PIE_NOTICE, "reload|configFile:%s not changed", configFile.c_str());
				return;
			}

			auto node = YAML::LoadFile(configFile);
			apie::CtxSingleton::get().resetYamlNode(node);
			apie::CtxSingleton::get().setConfigFileMTime(mtime);

			PIE_LOG("Exception/Exception", PIE_CYCLE_HOUR, PIE_NOTICE, "reload|configFile:%s changed", configFile.c_str());

		}
		catch (std::exception& e) {
			std::stringstream ss;
			ss << "reload|configFile:" << configFile << "|Unexpected exception: " << e.what();
			PIE_LOG("Exception/Exception", PIE_CYCLE_HOUR, PIE_ERROR, "%s: %s", "Exception", ss.str().c_str());
		}
	}

} // namespace APie
