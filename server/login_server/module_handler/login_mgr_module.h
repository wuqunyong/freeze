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

namespace apie {

class LoginMgrModule
{
public:
	static void init();
	static void ready();


public:
	// PUBSUB
	static void PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg);
	static void PubSub_serverPeerClose(const std::shared_ptr<::pubsub::SERVER_PEER_CLOSE>& msg);


	// CMD
	static void Cmd_natsPublish(::pubsub::LOGIC_CMD& cmd);


	// CLIENT OPCODE
	static apie::status::Status handleAccount(
		uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>&, std::shared_ptr<::login_msg::MSG_RESPONSE_ACCOUNT_LOGIN_L>&);
	static void handleAccountNotify(uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>&);

};
}
