#include "module_handler/login_mgr_module.h"

#include "../../common/dao/model_account.h"
#include "../../common/dao/model_account_name.h"
#include "../../common/opcodes.h"


namespace apie {

void LoginMgrModule::init()
{
	// PUBSUB
	auto& pubsub = apie::pubsub::PubSubManagerSingleton::get();
	pubsub.subscribe<::pubsub::LOGIC_CMD>(::pubsub::PT_LogicCmd, LoginMgrModule::PubSub_logicCmd);
	pubsub.subscribe<::pubsub::SERVER_PEER_CLOSE>(::pubsub::PT_ServerPeerClose, LoginMgrModule::PubSub_serverPeerClose);


	// CMD
	auto& cmd = LogicCmdHandlerSingleton::get();
	cmd.init();
	cmd.registerOnCmd("nats_publish", "nats_publish", LoginMgrModule::Cmd_natsPublish);
}


void LoginMgrModule::ready()
{
	// CLIENT OPCODE
	using namespace ::login_msg;
	S_REGISTER_REQUEST(ACCOUNT_LOGIN_L, LoginMgrModule::handleAccount);
}


void LoginMgrModule::PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{
	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

void LoginMgrModule::PubSub_serverPeerClose(const std::shared_ptr<::pubsub::SERVER_PEER_CLOSE>& msg)
{
	std::stringstream ss;

	ss << "topic:" << ",refMsg:" << msg->ShortDebugString();
	ASYNC_PIE_LOG("LoginMgr/onServerPeerClose", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());
}

apie::status::Status LoginMgrModule::handleAccount(MessageInfo info, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_ACCOUNT_LOGIN_L>& response)
{
	std::stringstream ss;
	ss << "handleAccount:" << request->ShortDebugString();
	ASYNC_PIE_LOG("LoginMgr/handleAccount", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	ModelAccount accountData(request->account_id());
	bool bResult = accountData.checkInvalid();
	if (!bResult)
	{
		response->set_status_code(opcodes::SC_BindTable_Error);
		response->set_account_id(request->account_id());
		return { apie::status::StatusCode::OK, "" };
	}

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ACCOUNT_Proxy);
	server.set_id(1);


	auto cb = [info, request, server](status::Status status, ModelAccount account, uint32_t iRows) {
		if (!status.ok())
		{
			::login_msg::MSG_RESPONSE_ACCOUNT_LOGIN_L response;
			response.set_status_code(apie::toUnderlyingType(status.errorCode()));
			response.set_account_id(request->account_id());
			
			service::ServiceManager::sendResponse(info, response);
			return;
		}

		::login_msg::MSG_RESPONSE_ACCOUNT_LOGIN_L response;
		response.set_status_code(apie::toUnderlyingType(status.errorCode()));
		response.set_account_id(request->account_id());

		auto gatewayOpt = EndPointMgrSingleton::get().modulusEndpointById(::common::EPT_Gateway_Server, request->account_id());
		if (!gatewayOpt.has_value())
		{
			response.set_status_code(opcodes::SC_Discovery_ServerListEmpty);
			service::ServiceManager::sendResponse(info, response);
			return;
		}

		std::string ip = gatewayOpt.value().ip();
		uint32_t port = gatewayOpt.value().port();
		std::string sessionKey = apie::randomStr(16);

		response.set_ip(ip);
		response.set_port(port);
		response.set_session_key(sessionKey);

		::rpc_login::RPC_LoginPendingRequest rpcRequest;
		rpcRequest.set_account_id(request->account_id());
		rpcRequest.set_session_key(sessionKey);
		rpcRequest.set_db_id(account.fields.db_id);
		rpcRequest.set_version(request->version());

		if (iRows != 0)
		{
			auto iAccountId = request->account_id();

			auto curTime = apie::Ctx::getCurSeconds();
			account.fields.modified_time = curTime;

			account.markDirty({ ModelAccount::modified_time });
			auto cb = [iAccountId](status::Status status, bool result, uint64_t affectedRows) {
				if (!status.ok())
				{
					std::stringstream ss;
					ss << "UpdateToDb Error|accountId:" << iAccountId;
					ASYNC_PIE_LOG("LoginMgr/handleAccountLogin", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());
					return;
				}
			};
			UpdateToDb<ModelAccount>(server, account, cb);

			::rpc_msg::CHANNEL server;
			server.set_realm(apie::Ctx::getThisChannel().realm());
			server.set_type(gatewayOpt.value().type());
			server.set_id(gatewayOpt.value().id());

			auto rpcCB = [info, response](const apie::status::Status& status, const std::shared_ptr< rpc_login::RPC_LoginPendingResponse>& responsePtr) mutable {
				if (!status.ok())
				{
					response.set_status_code(apie::toUnderlyingType(status.errorCode()));
					service::ServiceManager::sendResponse(info, response);
					return;
				}

				service::ServiceManager::sendResponse(info, response);
			};
			apie::rpc::RPC_Call<::rpc_login::RPC_LoginPendingRequest, rpc_login::RPC_LoginPendingResponse>(server, ::rpc_msg::OP_RPC_LoginPending, rpcRequest, rpcCB);
			return;
		}

		auto roleDBopt = EndPointMgrSingleton::get().modulusEndpointById(::common::EPT_DB_ROLE_Proxy, request->account_id());
		if (!roleDBopt.has_value())
		{
			response.set_status_code(opcodes::SC_Discovery_ServerListEmpty);
			service::ServiceManager::sendResponse(info, response);
			return;
		}


		auto curTime = time(NULL);
		account.fields.register_time = curTime;
		account.fields.modified_time = curTime;
		account.fields.db_id = roleDBopt.value().id();

		auto cb = [info, response, gatewayOpt, rpcRequest](status::Status status, bool result, uint64_t affectedRows, uint64_t insertId) mutable {
			if (!status.ok())
			{
				response.set_status_code(apie::toUnderlyingType(status.errorCode()));
				service::ServiceManager::sendResponse(info, response);
				return;
			}

			::rpc_msg::CHANNEL server;
			server.set_realm(apie::Ctx::getThisChannel().realm());
			server.set_type(gatewayOpt.value().type());
			server.set_id(gatewayOpt.value().id());

			auto rpcCB = [info, response](const apie::status::Status& status, const std::shared_ptr< rpc_login::RPC_LoginPendingResponse>& responsePtr) mutable {
				if (!status.ok())
				{
					response.set_status_code(apie::toUnderlyingType(status.errorCode()));
					service::ServiceManager::sendResponse(info, response);
					return;
				}

				service::ServiceManager::sendResponse(info, response);
			};
			apie::rpc::RPC_Call<::rpc_login::RPC_LoginPendingRequest, rpc_login::RPC_LoginPendingResponse>(server, ::rpc_msg::OP_RPC_LoginPending, rpcRequest, rpcCB);
		};
		InsertToDb<ModelAccount>(server, account, cb);
	};
	LoadFromDb<ModelAccount>(server, accountData, cb);


	return { apie::status::StatusCode::OK_ASYNC, "" };
}

void LoginMgrModule::handleAccountNotify(MessageInfo info, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>& notify)
{
	std::stringstream ss;
	ss << "handleAccount:" << notify->ShortDebugString();
	ASYNC_PIE_LOG("LoginMgr/handleAccount", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());
}

void LoginMgrModule::Cmd_natsPublish(::pubsub::LOGIC_CMD& cmd)
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
	//APie::Event::NatsSingleton::get().publishNatsMsg(APie::Event::NatsManager::E_NT_Realm, channel, nats_msg);

	rpc_msg::RPC_EchoTestRequest params;
	params.set_value1(200);
	params.set_value2("test_hello");

	::rpc_msg::CHANNEL server;
	server.set_realm(realm);
	server.set_type(type);
	server.set_id(id);

	//auto rpcObj = apie::rpc::createRPCClient<rpc_msg::RPC_EchoTestRequest, rpc_msg::RPC_EchoTestResponse>(server, rpc_msg::OP_RPC_EchoTest, nullptr);
	//rpcObj->sendRequest(params);

	auto cb = [](const apie::status::Status& status, const std::shared_ptr<rpc_msg::RPC_EchoTestResponse>& response) {
		if (!status.ok())
		{
			return;
		}

		std::stringstream ss;
		ss << "RPC_echoCb:" << response->ShortDebugString();
	};
	apie::rpc::RPC_Call<rpc_msg::RPC_EchoTestRequest, rpc_msg::RPC_EchoTestResponse>(server, rpc_msg::OP_RPC_EchoTest, params, cb);

}

}

