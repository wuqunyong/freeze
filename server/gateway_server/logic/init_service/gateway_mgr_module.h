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
#include "pb_init.h"

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
		static void Cmd_natsPublish(::pubsub::LOGIC_CMD& cmd);
		static void Cmd_mysqlStatement(::pubsub::LOGIC_CMD& cmd);

		// RPC
		static apie::status::Status RPC_loginPending(
			const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<pb::rpc::RPC_LoginPendingRequest>& request, std::shared_ptr<pb::rpc::RPC_LoginPendingResponse>& response);


		// CLIENT OPCODE
		static void handleDefaultOpcodes(MessageInfo info, const std::string& msg);
		static void handleDemuxForward(const ::rpc_msg::RoleIdentifier& role, const std::string& msg);

		static apie::status::E_ReturnType handleRequestHandshakeInit(
			MessageInfo info, const std::shared_ptr<::login_msg::HandshakeInitRequest>& request, std::shared_ptr<::login_msg::HandshakeInitResponse>& response);
		static apie::status::E_ReturnType handleRequestHandshakeEstablished(
			MessageInfo info, const std::shared_ptr<::login_msg::HandshakeEstablishedRequest>& request, std::shared_ptr<::login_msg::HandshakeEstablishedResponse>& response);

		static apie::status::E_ReturnType handleRequestClientLogin(
			MessageInfo info, const std::shared_ptr<::login_msg::ClientLoginRequest>& request, std::shared_ptr<::login_msg::ClientLoginResponse>& response);
		static apie::status::E_ReturnType handleEcho(
			MessageInfo info, const std::shared_ptr<::login_msg::EchoRequest>& request, std::shared_ptr<::login_msg::EchoResponse>& response);
		static apie::status::E_ReturnType handleAsyncEcho(
			MessageInfo info, const std::shared_ptr<::login_msg::AsyncEchoRequest>& request, std::shared_ptr<::login_msg::AsyncEchoResponse>& response);
	};
}
