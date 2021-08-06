#pragma once

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <atomic>

#include "yaml-cpp/yaml.h"

#include "apie/singleton/threadsafe_singleton.h"
#include "apie/event/dispatched_thread.h"
#include "apie/network/platform_impl.h"
#include "apie/network/i_poll_events.hpp"
#include "apie/network/end_point.h"
#include "apie/configs/configs.h"
#include "apie/proto/init.h"


namespace apie
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

		std::shared_ptr<event_ns::DispatchedThreadImpl> chooseIOThread();
		std::shared_ptr<event_ns::DispatchedThreadImpl> getLogicThread();
		std::shared_ptr<event_ns::DispatchedThreadImpl> getLogThread();
		std::shared_ptr<event_ns::DispatchedThreadImpl> getMetricsThread();
		std::shared_ptr<event_ns::DispatchedThreadImpl> getNatsThread();

		//std::shared_ptr<Event::DispatchedThreadImpl> getDBThread();

		std::shared_ptr<event_ns::DispatchedThreadImpl> getThreadById(uint32_t id);

		std::string launchTime();

		uint32_t getServerRealm();
		void setServerRealm(uint32_t realm);

		uint32_t getServerId();
		void setServerId(uint32_t id);

		uint32_t getServerType();
		void setServerType(uint32_t type);

		bool checkIsValidServerType(std::set<uint32_t> validSet);

		std::string getConfigFile();
		int64_t getConfigFileMTime();
		void setConfigFileMTime(int64_t mtime);

		std::shared_ptr<APieConfig> getConfigs();

	public:
		static std::string logPostfix();
		
		static uint64_t getCurMilliseconds();
		static uint64_t getCurSeconds();
		static ::rpc_msg::CHANNEL getThisChannel();

    private:
		void daemonize();
		bool adjustOpenFilesLimit();
		void enableCoreFiles();
		void handleSigProcMask();

		std::shared_ptr<APieConfig> loadConfigs();

		typedef std::vector<std::shared_ptr<event_ns::DispatchedThreadImpl>> ThreadVec;
		std::map<event_ns::EThreadType, ThreadVec> thread_;

		std::shared_ptr<event_ns::DispatchedThreadImpl> logic_thread_;
		std::shared_ptr<event_ns::DispatchedThreadImpl> log_thread_;
		std::shared_ptr<event_ns::DispatchedThreadImpl> metrics_thread_;
		//std::shared_ptr<Event::DispatchedThreadImpl> db_thread_;
		std::shared_ptr<event_ns::DispatchedThreadImpl> nats_thread_;

		std::map<uint32_t, std::shared_ptr<event_ns::DispatchedThreadImpl>> thread_id_;

		std::string m_launchTime;

		std::atomic<uint32_t> tid_ = 0;
		YAML::Node node_;
		std::mutex node_sync_;

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

