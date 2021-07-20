#pragma once

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <atomic>

#include "../singleton/threadsafe_singleton.h"
#include "../event/dispatched_thread.h"
#include "../network/platform_impl.h"
#include "../network/i_poll_events.hpp"
#include "../network/end_point.h"

#include "../configs/configs.h"

#include "apie/proto/init.h"

#include "yaml-cpp/yaml.h"

namespace APie
{
    //  Context object encapsulates all the global state associated with
    //  the library.

    class Ctx
    {
    public:
		Ctx();
		~Ctx();

		EndPoint identify();
		std::shared_ptr<SelfRegistration> getEndpoint();

		uint32_t generateHash(EndPoint point);

		void init(const std::string& configFile);
		void start();
		void destroy();

		void waitForShutdown();

		uint32_t generatorTId();

		void resetYamlNode(YAML::Node node);

		std::shared_ptr<Event::DispatchedThreadImpl> chooseIOThread();
		std::shared_ptr<Event::DispatchedThreadImpl> getLogicThread();
		std::shared_ptr<Event::DispatchedThreadImpl> getLogThread();
		std::shared_ptr<Event::DispatchedThreadImpl> getMetricsThread();
		std::shared_ptr<Event::DispatchedThreadImpl> getNatsThread();

		//std::shared_ptr<Event::DispatchedThreadImpl> getDBThread();

		std::shared_ptr<Event::DispatchedThreadImpl> getThreadById(uint32_t id);

		std::string launchTime();

		uint32_t getServerRealm();
		void setServerRealm(uint32_t realm);

		uint32_t getServerId();
		void setServerId(uint32_t id);

		uint32_t getServerType();
		void setServerType(uint32_t type);

		bool checkIsValidServerType(std::set<uint32_t> validSet);

		bool isDaemon();
		std::string getConfigFile();
		int64_t getConfigFileMTime();
		void setConfigFileMTime(int64_t mtime);

		std::shared_ptr<APieConfig> getConfigs();

	public:
		static std::string logPostfix();
		
		static uint64_t getCurMilliseconds();
		static uint64_t getCurSeconds();

    private:
		void daemonize();
		bool adjustOpenFilesLimit();
		void enableCoreFiles();
		void handleSigProcMask();

		std::shared_ptr<APieConfig> loadConfigs();

		typedef std::vector<std::shared_ptr<Event::DispatchedThreadImpl>> ThreadVec;
		std::map<Event::EThreadType, ThreadVec> thread_;

		std::shared_ptr<Event::DispatchedThreadImpl> logic_thread_;
		std::shared_ptr<Event::DispatchedThreadImpl> log_thread_;
		std::shared_ptr<Event::DispatchedThreadImpl> metrics_thread_;
		//std::shared_ptr<Event::DispatchedThreadImpl> db_thread_;
		std::shared_ptr<Event::DispatchedThreadImpl> nats_thread_;

		std::map<uint32_t, std::shared_ptr<Event::DispatchedThreadImpl>> thread_id_;

		std::string m_launchTime;

		std::atomic<uint32_t> tid_ = 0;
		YAML::Node node_;
		std::mutex node_sync_;

		bool m_bDaemon = true;
		std::string m_configFile;
		int64_t m_configFileMTime = -1;

		std::shared_ptr<SelfRegistration> endpoint_ = nullptr;

		uint32_t m_server_realm = 0;
		uint32_t m_server_id = 0;
		uint32_t m_server_type = 0;

		std::shared_ptr<APieConfig> m_ptrConfig;

        Ctx (const Ctx&) = delete;
        const Ctx &operator = (const Ctx&) = delete;

		static PlatformImpl s_platform;
		static std::string s_log_postfix;
    };
    

	using CtxSingleton = ThreadSafeSingleton<Ctx>;

}

