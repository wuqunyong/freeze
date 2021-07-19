#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"

#include "../../pb_msg/core/service_discovery.pb.h"


namespace APie {

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
		// CMD
		static void onLogicCommnad(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg);

		static void onShowProvider(::pubsub::LOGIC_CMD& cmd);

		// Inner Protocols		
		static apie::status::Status handleRequestRegisterInstance(uint64_t iSerialNum, const std::shared_ptr<::service_discovery::MSG_REQUEST_REGISTER_INSTANCE>& request, 
			std::shared_ptr<::service_discovery::MSG_RESP_REGISTER_INSTANCE>& response);
		static apie::status::Status handleRequestHeartbeat(uint64_t iSerialNum, const std::shared_ptr<::service_discovery::MSG_REQUEST_HEARTBEAT>& request,
			std::shared_ptr<::service_discovery::MSG_RESP_HEARTBEAT>& response);


		static void onServerPeerClose(const std::shared_ptr<::pubsub::SERVER_PEER_CLOSE>& msg);

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

	public:
		std::map<uint64_t, RegisteredEndPoint> m_registered;
		std::map<EndPoint, uint64_t> m_pointMap;
		uint32_t m_serviceTimeout = 300;

		std::string m_id;
		uint64_t m_version = 0;
		::service_discovery::RegistryStatus m_status = service_discovery::RS_Learning;
		uint64_t m_iStatusCheckTime = 0;
		Event::TimerPtr m_updateTimer;
	};


	//typedef APie::ThreadSafeSingleton<ServiceRegistry> ServiceRegistrySingleton;

	using ServiceRegistrySingleton = ThreadSafeSingleton<ServiceRegistry>;
}
