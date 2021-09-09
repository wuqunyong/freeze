#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"
#include "../../pb_msg/business/login_msg.pb.h"

namespace apie {

	class SceneMgrModule
	{
	public:
		static void init();
		static void ready();

	public:
		// PUBSUB
		static void PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg);


		// RPC
		static apie::status::Status RPC_echoTest(
			const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<rpc_msg::RPC_EchoTestRequest>& request, std::shared_ptr<rpc_msg::RPC_EchoTestResponse>& response);


		// FORWARD
		static apie::status::Status Forward_echo(
			const ::rpc_msg::RoleIdentifier& role, const std::shared_ptr<::login_msg::MSG_REQUEST_ECHO>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_ECHO>& response);
	};

}
