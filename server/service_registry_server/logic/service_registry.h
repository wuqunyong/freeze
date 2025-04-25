#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"

#include "../../common/dao/dbt_configdb/service_node_AutoGen.h"

namespace apie {

	struct RegisteredEndPoint
	{
		uint64_t addTime = {0};
		uint64_t modifyTime = {0};
		::service_discovery::EndPointInstance instance;
	};

	class ServiceRegistry
	{
	public:
		ServiceRegistry(std::string name, module_loader::ModuleLoaderBase* prtLoader);

		static std::string moduleName();
		static uint32_t modulePrecedence();

		apie::status::Status init();
		apie::status::Status start();
		apie::status::Status ready();
		apie::status::Status exit();

		void setHookReady(hook::HookPoint point);

	public:
		void update();

		bool updateInstance(uint64_t iSerialNum, const ::service_discovery::EndPointInstance& instance);
		bool updateHeartbeat(uint64_t iSerialNum);
		bool deleteBySerialNum(uint64_t iSerialNum);
		bool deleteByNats(EndPoint point);

		bool updateNatsInstance(const ::service_discovery::EndPointInstance& instance);

		void checkTimeout();
		void broadcast();

		void addUpdateTimer(uint64_t interval);
		void disableUpdateTimer();

		std::map<uint64_t, RegisteredEndPoint>& registered();
		std::optional<dbt_configdb::service_node_AutoGen> findNode(EndPoint key);

	public:
		std::string m_name;
		module_loader::ModuleLoaderBase* m_prtLoader;

		std::map<uint64_t, RegisteredEndPoint> m_registered;        // 通过TCP连接注册的节点
		std::map<EndPoint, RegisteredEndPoint> m_natsRegistered;    // 通过NATS注册的节点

		std::map<EndPoint, uint64_t> m_pointMap;  //已注册的节点                
		uint32_t m_serviceTimeout = 300;

		std::string m_id;
		uint64_t m_version = 0;
		::service_discovery::RegistryStatus m_status = service_discovery::RS_Learning;
		uint64_t m_iStatusCheckTime = 0;
		event_ns::TimerPtr m_updateTimer;

		std::map<EndPoint, dbt_configdb::service_node_AutoGen> m_nodes;
	};

}
