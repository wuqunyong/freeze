#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"
#include "../../../pb_msg/business/login_msg.pb.h"
#include "../../../pb_msg/business/rpc_login.pb.h"

namespace apie {

	class GatewayMgrModule
	{
	public:
		static void init();
		static void ready();

	public:
		// PUBSUB
		static void PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg);
		static void PubSub_serverPeerClose(const std::shared_ptr<::pubsub::SERVER_PEER_CLOSE>& msg);


		// CMD
		static void Cmd_insertToDbORM(::pubsub::LOGIC_CMD& cmd);
		static void Cmd_deleteFromDbORM(::pubsub::LOGIC_CMD& cmd);
		static void Cmd_updateToDbORM(::pubsub::LOGIC_CMD& cmd);
		static void Cmd_loadFromDbORM(::pubsub::LOGIC_CMD& cmd);
		static void Cmd_queryFromDbORM(::pubsub::LOGIC_CMD& cmd);
		static void Cmd_natsPublish(::pubsub::LOGIC_CMD& cmd);
		

		// RPC
		static apie::status::Status RPC_loginPending(
			const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<rpc_login::L2G_LoginPendingRequest>& request, std::shared_ptr<rpc_login::L2G_LoginPendingResponse>& response);


		// CLIENT OPCODE
		static void handleDefaultOpcodes(uint64_t serialNum, uint32_t opcodes, const std::string& msg);
		static void handleDemuxForward(const ::rpc_msg::RoleIdentifier& role, uint32_t opcode, const std::string& msg);

		static apie::status::Status handleRequestClientLogin(
			uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_CLIENT_LOGIN>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_CLIENT_LOGIN>& response);
		static apie::status::Status handleRequestHandshakeInit(
			uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_HANDSHAKE_INIT>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_HANDSHAKE_INIT>& response);
		static apie::status::Status handleRequestHandshakeEstablished(
			uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_HANDSHAKE_ESTABLISHED>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_HANDSHAKE_ESTABLISHED>& response);
	};
}
