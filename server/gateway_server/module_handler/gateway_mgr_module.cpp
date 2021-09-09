#include "module_handler/gateway_mgr_module.h"

#include <type_traits>

#include "../../common/dao/model_user.h"
#include "../../common/dao/model_role_extra.h"
#include "../../common/opcodes.h"

#include "logic/gateway_mgr.h"
#include "logic/gateway_role.h"

namespace apie {


void GatewayMgrModule::init()
{	
	// PUBSUB
	auto& pubsub = apie::pubsub::PubSubManagerSingleton::get();
	pubsub.subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, GatewayMgrModule::PubSub_logicCmd);
	pubsub.subscribe<::pubsub::SERVER_PEER_CLOSE>(::pubsub::PT_ServerPeerClose, GatewayMgrModule::PubSub_serverPeerClose);

	// CMD
	auto& cmd = LogicCmdHandlerSingleton::get();
	cmd.init();
	cmd.registerOnCmd("insert_to_db", "mysql_insert_to_db_orm", GatewayMgrModule::Cmd_insertToDbORM);
	cmd.registerOnCmd("delete_from_db", "mysql_delete_from_db_orm", GatewayMgrModule::Cmd_deleteFromDbORM);
	cmd.registerOnCmd("update_to_db", "mysql_update_to_db_orm", GatewayMgrModule::Cmd_updateToDbORM);
	cmd.registerOnCmd("load_from_db", "mysql_load_from_db_orm", GatewayMgrModule::Cmd_loadFromDbORM);
	cmd.registerOnCmd("query_from_db", "mysql_query_from_db_orm", GatewayMgrModule::Cmd_queryFromDbORM);
	cmd.registerOnCmd("mulit_load_from_db", "mysql_mulit_load_from_db_orm", GatewayMgrModule::Cmd_multiLoadFromDbORM);


	cmd.registerOnCmd("nats_publish", "nats_publish", GatewayMgrModule::Cmd_natsPublish);
}

void GatewayMgrModule::ready()
{
	// RPC
	auto& rpc = apie::rpc::RPCServerManagerSingleton::get();
	rpc.createRPCServer<::rpc_login::L2G_LoginPendingRequest, ::rpc_login::L2G_LoginPendingResponse>(rpc_msg::RPC_L2G_LoginPending, GatewayMgrModule::RPC_loginPending);


	// CLIENT OPCODE
	auto& server = apie::service::ServiceHandlerSingleton::get().server;
	server.setDefaultFunc(GatewayMgrModule::handleDefaultOpcodes);

	using namespace ::login_msg;
	S_REGISTER_SERVICE(CLIENT_LOGIN, GatewayMgrModule::handleRequestClientLogin);
	S_REGISTER_SERVICE(HANDSHAKE_INIT, GatewayMgrModule::handleRequestHandshakeInit);
	S_REGISTER_SERVICE(HANDSHAKE_ESTABLISHED, GatewayMgrModule::handleRequestHandshakeEstablished);

	// FORWARD
	apie::forward::ForwardManagerSingleton::get().setDemuxCallback(GatewayMgrModule::handleDemuxForward);
}

void GatewayMgrModule::handleDefaultOpcodes(MessageInfo info, const std::string& msg)
{	
	auto ptrGatewayRole = GatewayMgrSingleton::get().findGatewayRoleBySerialNum(info.iSessionId);
	if (ptrGatewayRole == nullptr)
	{
		ASYNC_PIE_LOG("handleDefaultOpcodes", PIE_CYCLE_DAY, PIE_ERROR, "Not Login|serialNum:%lld|opcodes:%d", info.iSessionId, info.iOpcode);
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

	apie::forward::ForwardManagerSingleton::get().sendForwardMux(server, role, info.iOpcode, msg);
}

void GatewayMgrModule::handleDemuxForward(const ::rpc_msg::RoleIdentifier& role, uint32_t opcode, const std::string& msg)
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

apie::status::Status GatewayMgrModule::RPC_loginPending(
	const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<rpc_login::L2G_LoginPendingRequest>& request, std::shared_ptr<rpc_login::L2G_LoginPendingResponse>& response)
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

void GatewayMgrModule::PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{

	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

void GatewayMgrModule::Cmd_loadFromDbORM(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 1)
	{
		return;
	}

	uint64_t userId = std::stoull(cmd.params()[0]);

	ModelUser user(userId);
	bool bResult = user.checkInvalid();
	if (!bResult)
	{
		return;
	}

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [](status::Status status, ModelUser user, uint32_t iRows) {
		if (!status.ok())
		{
			return;
		}
	};
	LoadFromDb<ModelUser>(server, user, cb);
}

void GatewayMgrModule::Cmd_queryFromDbORM(::pubsub::LOGIC_CMD& cmd)
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
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [](status::Status status, std::vector<ModelUser>& userList) {
		if (!status.ok())
		{
			return;
		}
	};
	LoadFromDbByFilter<ModelUser>(server, user, cb);
}


template <size_t I = 0, typename... Ts>
void saveTuple(const ::rpc_msg::CHANNEL& server, std::tuple<Ts...>& tup, const std::array<uint32_t, sizeof...(Ts)>& rows,
	std::shared_ptr<std::tuple<uint32_t, uint32_t, bool>> ptrCheck, std::function<void(const status::Status& status, const std::tuple<uint32_t, uint32_t, bool>& check)> doneCb)
{
	// If we have iterated through all elements
	if constexpr (I == sizeof...(Ts))
	{
		// Last case, if nothing is left to
		// iterate, then exit the function

		if (std::get<0>(*ptrCheck) > std::get<1>(*ptrCheck))
		{
			return;
		}

		auto& doneFlag = std::get<2>(*ptrCheck);
		if (doneFlag)
		{
			return;
		}
		doneFlag = true;

		if (doneCb)
		{
			status::Status newStatus;
			doneCb(newStatus, *ptrCheck);
		}
		return;
	}
	else 
	{
		if (rows[I] == 0)
		{
			auto& pendingNum = std::get<0>(*ptrCheck);
			pendingNum += 1;

			auto cb = [ptrCheck, doneCb](status::Status status, bool result, uint64_t affectedRows, uint64_t insertId) mutable {
				auto& completedNum = std::get<1>(*ptrCheck);
				completedNum += 1;

				if (!status.ok())
				{
					auto& doneFlag = std::get<2>(*ptrCheck);
					if (doneFlag)
					{
						return;
					}
					doneFlag = true;

					if (doneCb)
					{
						doneCb(status, *ptrCheck);
					}

					return;
				}

				if (std::get<0>(*ptrCheck) > std::get<1>(*ptrCheck))
				{
					return;
				}

				auto& doneFlag = std::get<2>(*ptrCheck);
				if (doneFlag)
				{
					return;
				}
				doneFlag = true;

				if (doneCb)
				{
					doneCb(status, *ptrCheck);
				}
			};
			InsertToDb<std::tuple_element<I, std::decay_t<decltype(tup)>>::type>(server, std::get<I>(tup), cb);
		}

		// Going for next element.
		saveTuple<I + 1>(server, tup, rows, ptrCheck, doneCb);
	}
}

template <size_t I = 0, typename... Ts>
void saveTupleAux(const ::rpc_msg::CHANNEL& server, std::tuple<Ts...>& tup, const std::array<uint32_t, sizeof...(Ts)>& rows,
	std::function<void(const status::Status& status, const std::tuple<uint32_t, uint32_t, bool>& check)> doneCb)
{
	std::shared_ptr<std::tuple<uint32_t, uint32_t, bool>> ptrCheck = std::make_shared<std::tuple<uint32_t, uint32_t, bool>>(0, 0, false);
	saveTuple(server, tup, rows, ptrCheck, doneCb);
}


void GatewayMgrModule::Cmd_multiLoadFromDbORM(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 1)
	{
		return;
	}

	uint64_t userId = std::stoull(cmd.params()[0]);

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto multiCb = [server](const status::Status& status, std::tuple<ModelUser, ModelRoleExtra>& tupleData, const std::array<uint32_t, 2>& tupleRows) {
		if (!status.ok())
		{
			return;
		}

		auto doneCb = [server, tupleData](const status::Status& status, const std::tuple<uint32_t, uint32_t>& insertRows) mutable {
			if (!status.ok())
			{
				return;
			}

			std::get<0>(tupleData).fields.level += 10;
			std::get<0>(tupleData).markDirty({ ModelUser::level });

			std::get<1>(tupleData).fields.extra_info += "10";
			std::get<1>(tupleData).markDirty({ ModelRoleExtra::extra_info });

			Update_OnChanged(server, tupleData);
			Update_OnChanged(server, tupleData);

			Update_OnForced(server, tupleData);
		};
		Insert_OnNotExists(server, tupleData, tupleRows, doneCb);
	};
	Multi_LoadFromDb(multiCb, server, ModelUser(userId), ModelRoleExtra(userId));
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

	::login_msg::MSG_REQUEST_ECHO request;
	request.set_value1(100);
	request.set_value2("hello world");

	MessageInfo msgInfo;
	msgInfo.iOpcode = ::apie::OP_MSG_REQUEST_ECHO;
	GatewayMgrModule::handleDefaultOpcodes(msgInfo, request.SerializeAsString());

}

void GatewayMgrModule::Cmd_updateToDbORM(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 2)
	{
		return;
	}

	uint64_t userId = std::stoull(cmd.params()[0]);
	uint32_t level = std::stoul(cmd.params()[1]);


	ModelUser user(userId);
	user.fields.level = level;
	bool bResult = user.checkInvalid();
	if (!bResult)
	{
		return;
	}
	user.markDirty({ 2 });

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [](status::Status status, bool result, uint64_t affectedRows) {
		if (!status.ok())
		{
			return;
		}
	};
	UpdateToDb<ModelUser>(server, user, cb);
}

void GatewayMgrModule::Cmd_insertToDbORM(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 2)
	{
		return;
	}

	uint64_t userId = std::stoull(cmd.params()[0]);
	uint32_t level = std::stoul(cmd.params()[1]);


	ModelUser user(userId);
	user.fields.level = level;
	bool bResult = user.checkInvalid();
	if (!bResult)
	{
		return;
	}

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [](status::Status status, bool result, uint64_t affectedRows, uint64_t insertId) {
		if (!status.ok())
		{
			return;
		}
	};
	InsertToDb<ModelUser>(server, user, cb);
}

void GatewayMgrModule::Cmd_deleteFromDbORM(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 1)
	{
		return;
	}

	uint64_t userId = std::stoull(cmd.params()[0]);

	ModelUser user(userId);
	bool bResult = user.checkInvalid();
	if (!bResult)
	{
		return;
	}

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [](status::Status status, bool result, uint64_t affectedRows) {
		if (!status.ok())
		{
			return;
		}
	};
	DeleteFromDb<ModelUser>(server, user, cb);
}

apie::status::Status GatewayMgrModule::handleRequestClientLogin(
	MessageInfo info, const std::shared_ptr<::login_msg::MSG_REQUEST_CLIENT_LOGIN>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_CLIENT_LOGIN>& response)
{
	ModelUser user(request->user_id());
	bool bResult = user.checkInvalid();
	if (!bResult)
	{
		response->set_status_code(opcodes::SC_BindTable_Error);
		response->set_user_id(request->user_id());
		response->set_version(request->version());
		return { apie::status::StatusCode::OK, "" };
	}

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto cb = [info, request, server](status::Status status, ModelUser user, uint32_t iRows) {
		if (!status.ok())
		{
			::login_msg::MSG_RESPONSE_CLIENT_LOGIN response;
			response.set_status_code(apie::toUnderlyingType(status.errorCode()));
			response.set_user_id(request->user_id());
			response.set_version(request->version());
			network::OutputStream::sendMsg(info.iSessionId, apie::OP_MSG_RESPONSE_CLIENT_LOGIN, response);
			return;
		}

		::login_msg::MSG_RESPONSE_CLIENT_LOGIN response;
		response.set_status_code(apie::toUnderlyingType(status.errorCode()));
		response.set_user_id(request->user_id());
		response.set_version(request->version());
		response.set_ammo(rand() % 30 + 50);
		response.set_grenades(rand() % 10 + 20);
		if (iRows == 0)
		{
			response.set_is_newbie(true);

			auto cb = [](status::Status status, bool result, uint64_t affectedRows, uint64_t insertId) mutable {
				if (!status.ok())
				{
					return;
				}
			};
			InsertToDb<ModelUser>(server, user, cb);
		} 
		else
		{
			response.set_is_newbie(false);
		}

		auto ptrGatewayRole = GatewayRole::createGatewayRole(user.fields.user_id, info.iSessionId);
		GatewayMgrSingleton::get().addGatewayRole(ptrGatewayRole);

		network::OutputStream::sendMsg(info.iSessionId, apie::OP_MSG_RESPONSE_CLIENT_LOGIN, response);
	};
	LoadFromDb<ModelUser>(server, user, cb);

	return { apie::status::StatusCode::OK_ASYNC, "" };
}

apie::status::Status GatewayMgrModule::handleRequestHandshakeInit(
	MessageInfo info, const std::shared_ptr<::login_msg::MSG_REQUEST_HANDSHAKE_INIT>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_HANDSHAKE_INIT>& response)
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

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status GatewayMgrModule::handleRequestHandshakeEstablished(
	MessageInfo info, const std::shared_ptr<::login_msg::MSG_REQUEST_HANDSHAKE_ESTABLISHED>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_HANDSHAKE_ESTABLISHED>& response)
{
	std::string decryptedMsg;
	bool bResult = apie::crypto::RSAUtilitySingleton::get().decrypt(request->encrypted_key(), &decryptedMsg);
	if (!bResult)
	{
		response->set_status_code(opcodes::SC_Auth_DecryptError);
		return { apie::status::StatusCode::OK, "" };
	}

	auto ptrConnection = event_ns::DispatcherImpl::getConnection(info.iSessionId);
	if (ptrConnection == nullptr)
	{
		response->set_status_code(opcodes::SC_Connection_Lost);
		return { apie::status::StatusCode::OK, "" };
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
	
	return { apie::status::StatusCode::OK, "" };;
}

void GatewayMgrModule::PubSub_serverPeerClose(const std::shared_ptr<::pubsub::SERVER_PEER_CLOSE>& msg)
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

