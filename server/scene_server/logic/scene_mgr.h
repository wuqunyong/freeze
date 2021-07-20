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

	class SceneMgr
	{
	public:
		apie::status::Status init();
		apie::status::Status start();
		apie::status::Status ready();
		void exit();

	public:
		// CMD
		static void onLogicCommnad(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg);



		// CLIENT OPCODE
		static void Forward_handlEcho(::rpc_msg::RoleIdentifier roleIdentifier, ::login_msg::MSG_REQUEST_ECHO request);
		static apie::status::Status RPC_echo(
			const ::rpc_msg::CLIENT_IDENTIFIER&, const std::shared_ptr<rpc_msg::MSG_RPC_REQUEST_ECHO>& request, std::shared_ptr<rpc_msg::MSG_RPC_RESPONSE_ECHO>& response);

	private:

	};

	using SceneMgrSingleton = ThreadSafeSingleton<SceneMgr>;
}
