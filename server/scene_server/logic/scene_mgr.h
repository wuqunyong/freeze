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

namespace APie {

	class SceneMgr
	{
	public:
		std::tuple<uint32_t, std::string> init();
		std::tuple<uint32_t, std::string> start();
		std::tuple<uint32_t, std::string> ready();
		void exit();

	public:
		// CMD
		static void onLogicCommnad(uint64_t topic, ::google::protobuf::Message& msg);



		// CLIENT OPCODE
		static void Forward_handlEcho(::rpc_msg::RoleIdentifier roleIdentifier, ::login_msg::MSG_REQUEST_ECHO request);
		static apie::status::Status RPC_echo(const ::rpc_msg::CLIENT_IDENTIFIER&, const std::shared_ptr<rpc_msg::MSG_RPC_REQUEST_ECHO>& request, std::shared_ptr<rpc_msg::MSG_RPC_RESPONSE_ECHO>& response);

	private:

	};

	using SceneMgrSingleton = ThreadSafeSingleton<SceneMgr>;
}
