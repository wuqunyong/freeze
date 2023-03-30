#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"


namespace apie {


class ServiceRegistryModule
{
public:
	static void init();
	static void ready();

public:
	// PUBSUB
	static void PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg);
	static void PubSub_serverPeerClose(const std::shared_ptr<::pubsub::SERVER_PEER_CLOSE>& msg);

	// CMD
	static void Cmd_showProvider(::pubsub::LOGIC_CMD& cmd);

	// Inner Protocols		
	static apie::status::E_ReturnType handleRequestRegisterInstance(MessageInfo info, const std::shared_ptr<::service_discovery::MSG_REQUEST_REGISTER_INSTANCE>& request,
		std::shared_ptr<::service_discovery::MSG_RESPONSE_REGISTER_INSTANCE>& response);
	static apie::status::E_ReturnType handleRequestHeartbeat(MessageInfo info, const std::shared_ptr<::service_discovery::MSG_REQUEST_HEARTBEAT>& request,
		std::shared_ptr<::service_discovery::MSG_RESPONSE_HEARTBEAT>& response);

};


}
