#include "gateway_mgr.h"

#include "../../common/dao/model_user.h"
#include "../../common/opcodes.h"

#include "gateway_role.h"

#include "../../apie/common/message_traits.h"
#include "../../apie/common/file.h"


namespace apie {


apie::status::Status GatewayMgr::init()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_Gateway_Server });
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}

	std::string pubKey = apie::CtxSingleton::get().getConfigs()->certificate.public_key;
	std::string privateKey = apie::CtxSingleton::get().getConfigs()->certificate.private_key;

	std::string errInfo;
	bResult = apie::crypto::RSAUtilitySingleton::get().init(pubKey, privateKey, errInfo);
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, errInfo };
	}
	
	// CMD
	apie::pubsub::PubSubManagerSingleton::get().subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, GatewayMgr::onLogicCommnad);

	LogicCmdHandlerSingleton::get().init();
	LogicCmdHandlerSingleton::get().registerOnCmd("insert_to_db", "mysql_insert_to_db_orm", GatewayMgr::onMysqlInsertToDbORM);
	LogicCmdHandlerSingleton::get().registerOnCmd("delete_from_db", "mysql_delete_from_db_orm", GatewayMgr::onMysqlDeleteFromDbORM);
	LogicCmdHandlerSingleton::get().registerOnCmd("update_to_db", "mysql_update_to_db_orm", GatewayMgr::onMysqlUpdateToDbORM);
	LogicCmdHandlerSingleton::get().registerOnCmd("load_from_db", "mysql_load_from_db_orm", GatewayMgr::onMysqlLoadFromDbORM);
	LogicCmdHandlerSingleton::get().registerOnCmd("query_from_db", "mysql_query_from_db_orm", GatewayMgr::onMysqlQueryFromDbORM);
	
	LogicCmdHandlerSingleton::get().registerOnCmd("nats_publish", "nats_publish", GatewayMgr::onNatsPublish);


	return { apie::status::StatusCode::OK, "" };
}


apie::status::Status GatewayMgr::start()
{
	// 加载:数据表结构
	auto dbType = DeclarativeBase::DBType::DBT_Role;
	auto ptrReadyCb = [](bool bResul, std::string sInfo, uint64_t iCallCount) {
		if (!bResul)
		{
			std::stringstream ss;
			ss << "CallMysqlDescTable|bResul:" << bResul << ",sInfo:" << sInfo << ",iCallCount:" << iCallCount;

			PANIC_ABORT(ss.str().c_str());
		}

		apie::hook::HookRegistrySingleton::get().triggerHook(hook::HookPoint::HP_Ready);

	};

	bool bResult = RegisterRequiredTable(dbType, 1, { {ModelUser::getFactoryName(), ModelUser::createMethod} }, ptrReadyCb);
	if (bResult)
	{
		apie::hook::HookRegistrySingleton::get().triggerHook(hook::HookPoint::HP_Ready);
		return { apie::status::StatusCode::OK, "" };
	}
	else
	{
		return { apie::status::StatusCode::HOOK_ERROR, "HR_Error" };
	}
}

apie::status::Status GatewayMgr::ready()
{
	// PubSub
	auto& pubsub = apie::pubsub::PubSubManagerSingleton::get();
	pubsub.subscribe<::pubsub::SERVER_PEER_CLOSE>(::pubsub::PT_ServerPeerClose, GatewayMgr::onServerPeerClose);


	// RPC
	auto& rpc = apie::rpc::RPCServerManagerSingleton::get();
	rpc.createRPCServer<::rpc_login::L2G_LoginPendingRequest, ::rpc_login::L2G_LoginPendingResponse>(rpc_msg::RPC_L2G_LoginPending, GatewayMgr::RPC_handleLoginPending);


	// CLIENT OPCODE
	auto& server = apie::service::ServiceHandlerSingleton::get().server;
	server.setDefaultFunc(GatewayMgr::handleDefaultOpcodes);
	server.createService<::login_msg::MSG_REQUEST_CLIENT_LOGIN, apie::OP_MSG_RESPONSE_CLIENT_LOGIN, ::login_msg::MSG_RESPONSE_CLIENT_LOGIN>(::apie::OP_MSG_REQUEST_CLIENT_LOGIN, GatewayMgr::handleRequestClientLogin);
	server.createService<::login_msg::MSG_REQUEST_HANDSHAKE_INIT, apie::OP_MSG_RESPONSE_HANDSHAKE_INIT, ::login_msg::MSG_RESPONSE_HANDSHAKE_INIT>(::apie::OP_MSG_REQUEST_HANDSHAKE_INIT, GatewayMgr::handleRequestHandshakeInit);
	server.createService<::login_msg::MSG_REQUEST_HANDSHAKE_ESTABLISHED, apie::OP_MSG_RESPONSE_HANDSHAKE_ESTABLISHED, ::login_msg::MSG_RESPONSE_HANDSHAKE_ESTABLISHED>(::apie::OP_MSG_REQUEST_HANDSHAKE_ESTABLISHED, GatewayMgr::handleRequestHandshakeEstablished);

	// FORWARD
	apie::forward::ForwardManagerSingleton::get().setDemuxCallback(GatewayMgr::handleDemuxForward);


	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ServerStatus", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void GatewayMgr::exit()
{

}

std::shared_ptr<GatewayRole> GatewayMgr::findGatewayRoleById(uint64_t iRoleId)
{
	auto findIte = m_roleIdMapSerialNum.find(iRoleId);
	if (findIte == m_roleIdMapSerialNum.end())
	{
		return nullptr;
	}

	return findGatewayRoleBySerialNum(findIte->second);
}

std::shared_ptr<GatewayRole> GatewayMgr::findGatewayRoleBySerialNum(uint64_t iSerialNum)
{
	auto findIte = m_serialNumMap.find(iSerialNum);
	if (findIte == m_serialNumMap.end())
	{
		return nullptr;
	}

	return findIte->second;
}

std::optional<uint64_t> GatewayMgr::findRoleIdBySerialNum(uint64_t iSerialNum)
{
	auto findIte = m_serialNumMap.find(iSerialNum);
	if (findIte == m_serialNumMap.end())
	{
		return std::nullopt;
	}

	return findIte->second->getRoleId();
}

void GatewayMgr::addPendingRole(const PendingLoginRole &role)
{
	m_pendingRole[role.role_id] = role;
}

std::optional<PendingLoginRole> GatewayMgr::getPendingRole(uint64_t iRoleId)
{
	auto findIte = m_pendingRole.find(iRoleId);
	if (findIte == m_pendingRole.end())
	{
		return std::nullopt;
	}

	return findIte->second;
}

void GatewayMgr::removePendingRole(uint64_t iRoleId)
{
	m_pendingRole.erase(iRoleId);
}

bool GatewayMgr::addGatewayRole(std::shared_ptr<GatewayRole> ptrGatewayRole)
{
	if (ptrGatewayRole == nullptr)
	{
		return false;
	}

	auto iRoleId = ptrGatewayRole->getRoleId();
	auto iSerialNum = ptrGatewayRole->getSerailNum();

	auto ptrConnection = event_ns::DispatcherImpl::getConnection(iSerialNum);
	if (ptrConnection != nullptr)
	{
		ptrGatewayRole->setMaskFlag(ptrConnection->getMaskFlag());
	}

	auto ptrExist = findGatewayRoleById(iRoleId);
	if (ptrExist != nullptr)
	{
		return false;
	}

	m_serialNumMap[iSerialNum] = ptrGatewayRole;
	m_roleIdMapSerialNum[iRoleId] = iSerialNum;

	return true;
}

bool GatewayMgr::removeGateWayRole(uint64_t iRoleId)
{
	auto findIte = m_roleIdMapSerialNum.find(iRoleId);
	if (findIte == m_roleIdMapSerialNum.end())
	{
		return false;
	}

	m_serialNumMap.erase(findIte->second);
	m_roleIdMapSerialNum.erase(findIte);
	return true;
}

void GatewayMgr::handleDefaultOpcodes(uint64_t serialNum, uint32_t opcodes, const std::string& msg)
{	
	//auto ptrGatewayRole = GatewayMgrSingleton::get().findGatewayRoleBySerialNum(serialNum);
	//if (ptrGatewayRole == nullptr)
	//{
	//	ASYNC_PIE_LOG("handleDefaultOpcodes", PIE_CYCLE_DAY, PIE_ERROR, "Not Login|serialNum:%lld|opcodes:%d", serialNum, opcodes);
	//	return;
	//}

	//uint64_t iUserId = ptrGatewayRole->getRoleId();

	::rpc_msg::CHANNEL server;
	server.set_realm(1);
	server.set_type(4);
	server.set_id(1);

	::rpc_msg::RoleIdentifier role;
	*role.mutable_gw_id() = apie::Ctx::getThisChannel();
	role.set_user_id(100);

	apie::forward::ForwardManagerSingleton::get().sendForwardMux(server, role, opcodes, msg);
}

void GatewayMgr::handleDemuxForward(const ::rpc_msg::RoleIdentifier& role, uint32_t opcode, const std::string& msg)
{
	uint64_t iRoleId = role.user_id();
	auto ptrGatewayRole = GatewayMgrSingleton::get().findGatewayRoleById(iRoleId);
	if (ptrGatewayRole == nullptr)
	{
		return;
	}

	uint64_t iSerialNum = ptrGatewayRole->getSerailNum();
	uint32_t iMaskFlag = ptrGatewayRole->getMaskFlag();
	if (iMaskFlag == 0)
	{
		network::OutputStream::sendMsgByStr(iSerialNum, opcode, msg, apie::ConnetionType::CT_SERVER);
	}
	else
	{
		network::OutputStream::sendMsgByStrByFlag(iSerialNum, opcode, msg, iMaskFlag, apie::ConnetionType::CT_SERVER);
	}
}

apie::status::Status GatewayMgr::RPC_handleLoginPending(
	const ::rpc_msg::CLIENT_IDENTIFIER& context, const std::shared_ptr<rpc_login::L2G_LoginPendingRequest>& request, std::shared_ptr<rpc_login::L2G_LoginPendingResponse>& response)
{
	auto curTime = apie::Ctx::getCurSeconds();

	PendingLoginRole role;
	role.role_id = request->account_id();
	role.session_key = request->session_key();
	role.db_id = request->db_id();
	role.expires_at = curTime + 30;

	GatewayMgrSingleton::get().addPendingRole(role);

	response->set_status_code(opcodes::SC_Ok);
	response->set_account_id(request->account_id());

	return { apie::status::StatusCode::OK, "" };
}

void GatewayMgr::onLogicCommnad(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{

	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

void GatewayMgr::onMysqlLoadFromDbORM(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 1)
	{
		return;
	}

	uint64_t userId = std::stoull(cmd.params()[0]);

	ModelUser user;
	user.fields.user_id = userId;

	bool bResult = user.bindTable(DeclarativeBase::DBType::DBT_Role, ModelUser::getFactoryName());
	if (!bResult)
	{
		return;
	}

	::rpc_msg::CHANNEL server;
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [](rpc_msg::STATUS status, ModelUser user, uint32_t iRows) {
		if (status.code() != ::rpc_msg::CODE_Ok)
		{
			return;
		}
	};
	LoadFromDb<ModelUser>(server, user, cb);
}

void GatewayMgr::onMysqlQueryFromDbORM(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 2)
	{
		return;
	}

	uint64_t gameId = std::stoull(cmd.params()[0]);
	uint32_t level = std::stoul(cmd.params()[1]);


	ModelUser user;
	user.fields.game_id = gameId;
	user.fields.level = level;
	bool bResult = user.bindTable(DeclarativeBase::DBType::DBT_Role, ModelUser::getFactoryName());
	if (!bResult)
	{
		return;
	}
	user.markFilter({ 1, 2 });

	::rpc_msg::CHANNEL server;
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [](rpc_msg::STATUS status, std::vector<ModelUser>& userList) {
		if (status.code() != ::rpc_msg::CODE_Ok)
		{
			return;
		}
	};
	LoadFromDbByFilter<ModelUser>(server, user, cb);
}

void GatewayMgr::onNatsPublish(::pubsub::LOGIC_CMD& cmd)
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

	::login_msg::MSG_REQUEST_ECHO request;
	request.set_value1(100);
	request.set_value2("hello world");

	GatewayMgr::handleDefaultOpcodes(0, ::apie::OP_MSG_REQUEST_ECHO, request.SerializeAsString());

}

void GatewayMgr::onMysqlUpdateToDbORM(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 2)
	{
		return;
	}

	uint64_t userId = std::stoull(cmd.params()[0]);
	uint32_t level = std::stoul(cmd.params()[1]);


	ModelUser user;
	user.fields.user_id = userId;
	user.fields.level = level;
	bool bResult = user.bindTable(DeclarativeBase::DBType::DBT_Role, ModelUser::getFactoryName());
	if (!bResult)
	{
		return;
	}
	user.markDirty({ 2 });

	::rpc_msg::CHANNEL server;
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [](rpc_msg::STATUS status, bool result, uint64_t affectedRows) {
		if (status.code() != ::rpc_msg::CODE_Ok)
		{
			return;
		}
	};
	UpdateToDb<ModelUser>(server, user, cb);
}

void GatewayMgr::onMysqlInsertToDbORM(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 2)
	{
		return;
	}

	uint64_t userId = std::stoull(cmd.params()[0]);
	uint32_t level = std::stoul(cmd.params()[1]);


	ModelUser user;
	user.fields.user_id = userId;
	user.fields.level = level;
	bool bResult = user.bindTable(DeclarativeBase::DBType::DBT_Role, ModelUser::getFactoryName());
	if (!bResult)
	{
		return;
	}

	::rpc_msg::CHANNEL server;
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [](rpc_msg::STATUS status, bool result, uint64_t affectedRows, uint64_t insertId) {
		if (status.code() != ::rpc_msg::CODE_Ok)
		{
			return;
		}
	};
	InsertToDb<ModelUser>(server, user, cb);
}

void GatewayMgr::onMysqlDeleteFromDbORM(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 1)
	{
		return;
	}

	uint64_t userId = std::stoull(cmd.params()[0]);

	ModelUser user;
	user.fields.user_id = userId;
	bool bResult = user.bindTable(DeclarativeBase::DBType::DBT_Role, ModelUser::getFactoryName());
	if (!bResult)
	{
		return;
	}

	::rpc_msg::CHANNEL server;
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [](rpc_msg::STATUS status, bool result, uint64_t affectedRows) {
		if (status.code() != ::rpc_msg::CODE_Ok)
		{
			return;
		}
	};
	DeleteFromDb<ModelUser>(server, user, cb);
}

apie::status::Status GatewayMgr::handleRequestClientLogin(
	uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_CLIENT_LOGIN>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_CLIENT_LOGIN>& response)
{
	ModelUser user;
	user.fields.user_id = request->user_id();

	bool bResult = user.bindTable(DeclarativeBase::DBType::DBT_Role, ModelUser::getFactoryName());
	if (!bResult)
	{
		response->set_status_code(opcodes::SC_BindTable_Error);
		response->set_user_id(request->user_id());
		response->set_version(request->version());
		return { apie::status::StatusCode::OK, "" };
	}

	::rpc_msg::CHANNEL server;
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [iSerialNum, request](rpc_msg::STATUS status, ModelUser user, uint32_t iRows) {
		if (status.code() != ::rpc_msg::CODE_Ok)
		{
			::login_msg::MSG_RESPONSE_CLIENT_LOGIN response;
			response.set_status_code(status.code());
			response.set_user_id(request->user_id());
			response.set_version(request->version());
			network::OutputStream::sendMsg(iSerialNum, apie::OP_MSG_RESPONSE_CLIENT_LOGIN, response);
			return;
		}

		::login_msg::MSG_RESPONSE_CLIENT_LOGIN response;
		response.set_status_code(status.code());
		response.set_user_id(request->user_id());
		response.set_version(request->version());
		if (iRows == 0)
		{
			response.set_is_newbie(true);
		} 
		else
		{
			response.set_is_newbie(false);
		}

		auto ptrGatewayRole = GatewayRole::createGatewayRole(user.fields.user_id, iSerialNum);
		GatewayMgrSingleton::get().addGatewayRole(ptrGatewayRole);

		network::OutputStream::sendMsg(iSerialNum, apie::OP_MSG_RESPONSE_CLIENT_LOGIN, response);
	};
	LoadFromDb<ModelUser>(server, user, cb);

	return { apie::status::StatusCode::OK_ASYNC, "" };
}

apie::status::Status GatewayMgr::handleRequestHandshakeInit(
	uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_HANDSHAKE_INIT>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_HANDSHAKE_INIT>& response)
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
	ptr->iSerialNum = iSerialNum;
	ptr->optClientRandom = request->client_random();
	ptr->optServerRandom = sServerRandom;

	Command cmd;
	cmd.type = Command::set_server_session_attr;
	cmd.args.set_server_session_attr.ptrData = ptr;
	network::OutputStream::sendCommand(ConnetionType::CT_SERVER, iSerialNum, cmd);

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status GatewayMgr::handleRequestHandshakeEstablished(
	uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_HANDSHAKE_ESTABLISHED>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_HANDSHAKE_ESTABLISHED>& response)
{
	std::string decryptedMsg;
	bool bResult = apie::crypto::RSAUtilitySingleton::get().decrypt(request->encrypted_key(), &decryptedMsg);
	if (!bResult)
	{
		response->set_status_code(opcodes::SC_Auth_DecryptError);
		return { apie::status::StatusCode::OK, "" };
	}

	auto ptrConnection = event_ns::DispatcherImpl::getConnection(iSerialNum);
	if (ptrConnection == nullptr)
	{
		return { apie::status::StatusCode::OK, "" };
	}

	std::string sClientRandom = ptrConnection->getClientRandom();
	std::string sServerRandom = ptrConnection->getServerRandom();

	std::string sSessionKey = sClientRandom + sServerRandom + decryptedMsg;

	response->set_status_code(opcodes::SC_Ok);

	apie::SetServerSessionAttr *ptr = new apie::SetServerSessionAttr;
	ptr->iSerialNum = iSerialNum;
	ptr->optKey = sSessionKey;

	Command cmd;
	cmd.type = Command::set_server_session_attr;
	cmd.args.set_server_session_attr.ptrData = ptr;
	network::OutputStream::sendCommand(ConnetionType::CT_SERVER, iSerialNum, cmd);
	
	return { apie::status::StatusCode::OK, "" };;
}

void GatewayMgr::onServerPeerClose(const std::shared_ptr<::pubsub::SERVER_PEER_CLOSE>& msg)
{
	std::stringstream ss;

	ss << "topic:" << ",refMsg:" << msg->ShortDebugString();
	ASYNC_PIE_LOG("GatewayMgr/onServerPeerClose", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	uint64_t iSerialNum = msg->serial_num();

	auto optRoleId = GatewayMgrSingleton::get().findRoleIdBySerialNum(iSerialNum);
	if (!optRoleId.has_value())
	{
		return;
	}

	GatewayMgrSingleton::get().removeGateWayRole(optRoleId.value());
}

}

