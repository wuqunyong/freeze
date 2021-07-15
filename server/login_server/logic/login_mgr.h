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
#include "../../pb_msg/business/rpc_login.pb.h"

namespace APie {

	class LoginMgr
	{
	public:
		std::tuple<uint32_t, std::string> init();
		std::tuple<uint32_t, std::string> start();
		std::tuple<uint32_t, std::string> ready();
		void exit();

	public:
		// CMD
		static void onLogicCommnad(uint64_t topic, ::google::protobuf::Message& msg);

		// PubSub
		static void onServerPeerClose(uint64_t topic, ::google::protobuf::Message& msg);

		// CLIENT OPCODE
		static void handleAccountLogin(uint64_t iSerialNum, const ::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L& request);
		

		static apie::status::Status handleAccount(uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>&, std::shared_ptr<::login_msg::MSG_RESPONSE_ACCOUNT_LOGIN_L>&);
		static void handleAccountNotify(uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>&);

	private:
		std::array<int, 5> a;
	};

	using LoginMgrSingleton = ThreadSafeSingleton<LoginMgr>;
}
