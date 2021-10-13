#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"

#include "../../common/dao/model_service_node.h"

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
		apie::status::Status init();
		apie::status::Status start();
		apie::status::Status ready();
		void exit();

	public:
		void update();

		bool updateInstance(uint64_t iSerialNum, const ::service_discovery::EndPointInstance& instance);
		bool updateHeartbeat(uint64_t iSerialNum);
		bool deleteBySerialNum(uint64_t iSerialNum);

		void checkTimeout();
		void broadcast();

		void addUpdateTimer(uint64_t interval);
		void disableUpdateTimer();

		std::map<uint64_t, RegisteredEndPoint>& registered();
		std::optional<ModelServiceNode> findNode(EndPoint key);

	public:
		std::map<uint64_t, RegisteredEndPoint> m_registered;
		std::map<EndPoint, uint64_t> m_pointMap;
		uint32_t m_serviceTimeout = 300;

		std::string m_id;
		uint64_t m_version = 0;
		::service_discovery::RegistryStatus m_status = service_discovery::RS_Learning;
		uint64_t m_iStatusCheckTime = 0;
		event_ns::TimerPtr m_updateTimer;

		std::map<EndPoint, ModelServiceNode> m_nodes;
	};

	using ServiceRegistrySingleton = ThreadSafeSingleton<ServiceRegistry>;
}
