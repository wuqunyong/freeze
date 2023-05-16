#include "logic/init_service/gateway_mgr_module.h"

#include <type_traits>


#include "logic/init_service/gateway_mgr.h"
#include "logic/init_service/gateway_role.h"

namespace apie {


void GatewayMgrModule::init()
{	
	// PUBSUB
	auto& pubsub = apie::pubsub::PubSubManagerSingleton::get();
	pubsub.subscribe<::pubsub::LOGIC_CMD>(::pubsub::PT_LogicCmd, GatewayMgrModule::PubSub_logicCmd);
	pubsub.subscribe<::pubsub::SERVER_PEER_CLOSE>(::pubsub::PT_ServerPeerClose, GatewayMgrModule::PubSub_serverPeerClose);

	// CMD
	auto& cmd = LogicCmdHandlerSingleton::get();
	cmd.init();
	cmd.registerOnCmd("nats_publish", "nats_publish", GatewayMgrModule::Cmd_natsPublish);
	cmd.registerOnCmd("mysql_statement", "mysql_statement", GatewayMgrModule::Cmd_mysqlStatement);
}

void GatewayMgrModule::ready()
{
	// RPC
	using namespace ::pb::rpc;
	INTRA_REGISTER_RPC(LoginPending, GatewayMgrModule::RPC_loginPending);

	// CLIENT OPCODE
	auto& server = apie::service::ServiceHandlerSingleton::get().server;
	server.setDefaultFunc(GatewayMgrModule::handleDefaultOpcodes);

	using namespace ::login_msg;
	S_REGISTER_REQUEST(ClientLogin, GatewayMgrModule::handleRequestClientLogin);
	S_REGISTER_REQUEST(HandshakeInit, GatewayMgrModule::handleRequestHandshakeInit);
	S_REGISTER_REQUEST(HandshakeEstablished, GatewayMgrModule::handleRequestHandshakeEstablished);

	S_REGISTER_REQUEST(Echo, GatewayMgrModule::handleEcho);

	// FORWARD
	apie::forward::ForwardManagerSingleton::get().setDemuxCallback(GatewayMgrModule::handleDemuxForward);
}

void GatewayMgrModule::handleDefaultOpcodes(MessageInfo info, const std::string& msg)
{	
	auto ptrGatewayRole = APieGetModule<GatewayMgr>()->findGatewayRoleBySerialNum(info.iSessionId);
	if (ptrGatewayRole == nullptr)
	{
		ASYNC_PIE_LOG(PIE_ERROR, "handleDefaultOpcodes|Not Login|serialNum:{}|opcodes:{}", info.iSessionId, info.iOpcode);
		return;
	}

	uint64_t iUserId = ptrGatewayRole->getRoleId();

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(4);
	server.set_id(1);

	::rpc_msg::RoleIdentifier role;
	*role.mutable_gw_id() = apie::Ctx::getThisChannel();
	role.set_user_id(iUserId);

	apie::forward::ForwardManagerSingleton::get().sendForwardMux(server, role, info, msg);
}

void GatewayMgrModule::handleDemuxForward(const ::rpc_msg::RoleIdentifier& role, const std::string& msg)
{
	MessageInfo info = apie::forward::ForwardManager::extractMessageInfo(role);

	uint64_t iRoleId = role.user_id();
	auto ptrGatewayRole = APieGetModule<GatewayMgr>()->findGatewayRoleById(iRoleId);
	if (ptrGatewayRole == nullptr)
	{
		return;
	}

	uint64_t iSerialNum = ptrGatewayRole->getSerailNum();
	uint32_t iMaskFlag = ptrGatewayRole->getMaskFlag();

	info.iSessionId = iSerialNum;
	info.iConnetionType = ConnetionType::CT_SERVER;
	info.setFlags(iMaskFlag);
	network::OutputStream::sendStringMsgImpl(info, msg);
}

apie::status::Status GatewayMgrModule::RPC_loginPending(
	const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<pb::rpc::RPC_LoginPendingRequest>& request, std::shared_ptr<pb::rpc::RPC_LoginPendingResponse>& response)
{
	auto curTime = apie::Ctx::getCurSeconds();

	PendingLoginRole role;
	role.role_id = request->account_id();
	role.session_key = request->session_key();
	role.db_id = request->db_id();
	role.expires_at = curTime + 30;

	APieGetModule<GatewayMgr>()->addPendingRole(role);

	response->set_status_code(opcodes::SC_Ok);
	response->set_account_id(request->account_id());

	return { apie::status::StatusCode::OK, "" };
}

void GatewayMgrModule::PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{

	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

void GatewayMgrModule::Cmd_natsPublish(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 5)
	{
		return;
	}

	uint32_t realm = std::stoul(cmd.params()[0]);
	uint32_t type = std::stoul(cmd.params()[1]);
	uint32_t id = std::stoul(cmd.params()[2]);

	std::string channel = apie::event_ns::NatsManager::GetTopicChannel(realm, type, id);

	std::string name = cmd.params()[3];
	std::string info = cmd.params()[4];

	//::nats_msg::NATS_MSG_PRXOY nats_msg;
	//apie::event_ns::NatsSingleton::get().publishNatsMsg(apie::event_ns::NatsManager::E_NT_Realm, channel, nats_msg);

	::login_msg::EchoRequest request;
	request.set_value1(100);
	request.set_value2("hello world");

	MessageInfo msgInfo;
	msgInfo.iOpcode = ::pb::core::OP_EchoRequest;
	GatewayMgrModule::handleDefaultOpcodes(msgInfo, request.SerializeAsString());
}

void GatewayMgrModule::Cmd_mysqlStatement(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 1)
	{
		return;
	}

	std::string sStatement = cmd.params()[0];

	auto cb = [](const apie::status::Status& status, const std::shared_ptr<::mysql_proxy_msg::MysqlStatementResponse>& response) {
		if (status.ok())
		{
			ASYNC_PIE_LOG(PIE_NOTICE, "Cmd_mysqlStatement|{}", response->ShortDebugString());
		}
	};

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DbAccount_Proxy);
	server.set_id(1);

	ExecMysqlStatement(server, sStatement, cb);
}


apie::status::E_ReturnType GatewayMgrModule::handleRequestClientLogin(
	MessageInfo info, const std::shared_ptr<::login_msg::ClientLoginRequest>& request, std::shared_ptr<::login_msg::ClientLoginResponse>& response)
{
	auto iId = request->user_id();
	auto optData = APieGetModule<GatewayMgr>()->getPendingRole(iId);
	if (!optData.has_value())
	{
		response->set_error_code(pb::core::CONTROL_ERROR);
		return apie::status::E_ReturnType::kRT_Sync;
	}

	response->set_error_code(pb::core::OK);
	response->set_user_id(iId);
	response->set_ammo(100);
	response->set_grenades(100);

	return apie::status::E_ReturnType::kRT_Sync;
}

apie::status::E_ReturnType GatewayMgrModule::handleRequestHandshakeInit(
	MessageInfo info, const std::shared_ptr<::login_msg::HandshakeInitRequest>& request, std::shared_ptr<::login_msg::HandshakeInitResponse>& response)
{
	std::string content;
	std::string pubKey = apie::CtxSingleton::get().getConfigs()->certificate.public_key;
	bool bResult = apie::common::GetContent(pubKey, &content);

	std::string sServerRandom("server");

	if (bResult)
	{
		response->set_status_code(opcodes::SC_Ok);
	}
	else
	{
		response->set_status_code(opcodes::SC_Auth_LoadPubFileError);
	}
	response->set_server_random(sServerRandom);
	response->set_public_key(content);


	apie::SetServerSessionAttr *ptr = new apie::SetServerSessionAttr;
	ptr->iSerialNum = info.iSessionId;
	ptr->optClientRandom = request->client_random();
	ptr->optServerRandom = sServerRandom;

	Command cmd;
	cmd.type = Command::set_server_session_attr;
	cmd.args.set_server_session_attr.ptrData = ptr;
	network::OutputStream::sendCommand(ConnetionType::CT_SERVER, info.iSessionId, cmd);

	return apie::status::E_ReturnType::kRT_Sync;
}

apie::status::E_ReturnType GatewayMgrModule::handleRequestHandshakeEstablished(
	MessageInfo info, const std::shared_ptr<::login_msg::HandshakeEstablishedRequest>& request, std::shared_ptr<::login_msg::HandshakeEstablishedResponse>& response)
{
	std::string decryptedMsg;
	bool bResult = apie::crypto::RSAUtilitySingleton::get().decrypt(request->encrypted_key(), &decryptedMsg);
	if (!bResult)
	{
		response->set_status_code(opcodes::SC_Auth_DecryptError);
		return apie::status::E_ReturnType::kRT_Sync;
	}

	auto ptrConnection = event_ns::DispatcherImpl::getConnection(info.iSessionId);
	if (ptrConnection == nullptr)
	{
		response->set_status_code(opcodes::SC_Connection_Lost);
		return apie::status::E_ReturnType::kRT_Sync;
	}

	std::string sClientRandom = ptrConnection->getClientRandom();
	std::string sServerRandom = ptrConnection->getServerRandom();

	std::string sSessionKey = sClientRandom + sServerRandom + decryptedMsg;

	response->set_status_code(opcodes::SC_Ok);

	apie::SetServerSessionAttr *ptr = new apie::SetServerSessionAttr;
	ptr->iSerialNum = info.iSessionId;
	ptr->optKey = sSessionKey;

	Command cmd;
	cmd.type = Command::set_server_session_attr;
	cmd.args.set_server_session_attr.ptrData = ptr;
	network::OutputStream::sendCommand(ConnetionType::CT_SERVER, info.iSessionId, cmd);
	
	return apie::status::E_ReturnType::kRT_Sync;
}

apie::status::E_ReturnType GatewayMgrModule::handleEcho(
	MessageInfo info, const std::shared_ptr<::login_msg::EchoRequest>& request, std::shared_ptr<::login_msg::EchoResponse>& response)
{
	auto value1 = request->value1();
	auto value2 = request->value2();

	response->set_value1(value1);
	response->set_value2(value2);

	return apie::status::E_ReturnType::kRT_Sync;
}

void GatewayMgrModule::PubSub_serverPeerClose(const std::shared_ptr<::pubsub::SERVER_PEER_CLOSE>& msg)
{
	std::stringstream ss;

	ss << "topic:" << ",refMsg:" << msg->ShortDebugString();
	ASYNC_PIE_LOG(PIE_NOTICE, "GatewayMgr/onServerPeerClose|{}", ss.str().c_str());

	uint64_t iSerialNum = msg->serial_num();

	auto optRoleId = APieGetModule<GatewayMgr>()->findRoleIdBySerialNum(iSerialNum);
	if (!optRoleId.has_value())
	{
		return;
	}

	APieGetModule<GatewayMgr>()->removeGateWayRole(optRoleId.value());
}

}

