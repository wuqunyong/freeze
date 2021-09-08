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
#include "../../pb_msg/business/role_server_msg.pb.h"

#include "logic/mock_role.h"


namespace apie {


class TestServerModule
{
public:
	static void init();
	static void ready();

public:
	static void PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg);

	static void Cmd_client(::pubsub::LOGIC_CMD& cmd);
	static void Cmd_autoTest(::pubsub::LOGIC_CMD& cmd);

	static void handleDefaultOpcodes(MessageInfo info, const std::string& msg);
};


}
