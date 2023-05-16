#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"
#include "pb_init.h"

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
	static void Cmd_loadAccount(::pubsub::LOGIC_CMD& cmd);
	static void Cmd_CoMysqlLoad(::pubsub::LOGIC_CMD& cmd);
	
	static CoTaskVoid CO_MysqlLoad(int64_t iRoleId);


	// CLIENT OPCODE
	static apie::status::E_ReturnType handleAccountLogin(
		MessageInfo info, const std::shared_ptr<::login_msg::AccountLoginRequest>& request, std::shared_ptr<::login_msg::AccountLoginResponse>& response);
};
}
