#include "logic/init_service/login_mgr_module.h"

#include "common/dao/init.h"

#include "logic/account/account.h"
#include "logic/account/component_create.h"
#include "logic/account/component_name.h"

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
	cmd.registerOnCmd("load_account", "load_account", LoginMgrModule::Cmd_loadAccount);
	cmd.registerOnCmd("co_mysql_load", "co_mysql_load", LoginMgrModule::Cmd_CoMysqlLoad);
}


void LoginMgrModule::ready()
{
	// CLIENT OPCODE
	using namespace ::login_msg;
	S_REGISTER_REQUEST(AccountLogin, LoginMgrModule::handleAccountLogin);
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
	ASYNC_PIE_LOG(PIE_NOTICE, "LoginMgr/onServerPeerClose|{}", ss.str().c_str());
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

	pb::rpc::RPC_EchoTestRequest params;
	params.set_value1(200);
	params.set_value2("test_hello");

	::rpc_msg::CHANNEL server;
	server.set_realm(realm);
	server.set_type(type);
	server.set_id(id);

	//auto rpcObj = apie::rpc::createRPCClient<rpc_msg::RPC_EchoTestRequest, rpc_msg::RPC_EchoTestResponse>(server, rpc_msg::OP_RPC_EchoTest, nullptr);
	//rpcObj->sendRequest(params);

	auto cb = [](const apie::status::Status& status, const std::shared_ptr<pb::rpc::RPC_EchoTestResponse>& response) {
		if (!status.ok())
		{
			return;
		}

		std::stringstream ss;
		ss << "RPC_echoCb:" << response->ShortDebugString();
	};
	apie::rpc::RPC_Call<pb::rpc::RPC_EchoTestRequest, pb::rpc::RPC_EchoTestResponse>(server, pb::rpc::OP_RPC_EchoTest, params, cb);
}

void LoginMgrModule::Cmd_loadAccount(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 1)
	{
		return;
	}

	uint32_t iId = std::stoul(cmd.params()[0]);
	auto ptrLoader = AccountLoader::Find(iId);
	if (ptrLoader != nullptr)
	{
		ptrLoader->lookup<ComponentWrapper<Component_Name>>().saveToDb();
		return;
	}

	auto doneCb = [iId](apie::status::Status status, AccountLoader::LoaderPtr ptrModule) {
		if (status.ok())
		{
			AccountLoader::Add(ptrModule);

			auto ptrLoader = AccountLoader::Find(iId);
			ptrLoader->lookup<ComponentWrapper<Component_Create>>().TestFunc();
		}
	};
	AccountLoader::LoadFromDb(iId, doneCb);
}
class TestCOScope
{
public:
	TestCOScope(int a)
	{
		a_ = a;
		ASYNC_PIE_LOG(PIE_NOTICE, "TestCOScope | con:{}", a_);
	}
	~TestCOScope()
	{
		ASYNC_PIE_LOG(PIE_NOTICE, "TestCOScope | decon:{}", a_);
	}

	int32_t a_ = 0;
};

CoTaskVoid LoginMgrModule::CO_MysqlLoad(int64_t iRoleId)
{
	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DbAccount_Proxy);
	server.set_id(1);

	apie::dbt_account::account_AutoGen dbObj(iRoleId);
	mysql_proxy_msg::MysqlQueryRequest queryRequest;
	queryRequest = dbObj.generateQuery();

	auto ptrScope = std::make_shared<TestCOScope>(iRoleId);

	auto ptrAwait = MakeCoAwaitable<::mysql_proxy_msg::MysqlQueryRequest, ::mysql_proxy_msg::MysqlQueryResponse>(server, rpc_msg::RPC_MysqlQuery, queryRequest);
	auto response = co_await *ptrAwait;
	if (!response.ok())
	{
		co_return;
	}

	if (ptrScope != nullptr)
	{
		ptrScope = nullptr;
	}

	auto valueObj = response.value();
	ASYNC_PIE_LOG(PIE_NOTICE, "CO_MysqlLoad | valueObj:{}", valueObj.ShortDebugString());
}


void LoginMgrModule::Cmd_CoMysqlLoad(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 1)
	{
		return;
	}

	uint32_t iId = std::stoul(cmd.params()[0]);
	CO_MysqlLoad(iId);
}

apie::status::E_ReturnType LoginMgrModule::handleAccountLogin(
	MessageInfo info, const std::shared_ptr<::login_msg::AccountLoginRequest>& request, std::shared_ptr<::login_msg::AccountLoginResponse>& response)
{
	auto iId = request->account_id();
	auto doneCb = [iId, info, response](apie::status::Status status, AccountLoader::LoaderPtr ptrModule) mutable {
		if (status.ok())
		{
			auto gatewayOpt = EndPointMgrSingleton::get().modulusEndpointById(::common::EPT_Gateway_Server, iId);
			if (!gatewayOpt.has_value())
			{
				response->set_error_code(opcodes::SC_Discovery_ServerListEmpty);
				service::ServiceManager::sendResponse(info, *response);
				return;
			}

			std::string ip = gatewayOpt.value().ip();
			uint32_t port = gatewayOpt.value().port();
			std::string sessionKey = apie::randomStr(16);

			response->set_ip(ip);
			response->set_port(port);
			response->set_session_key(sessionKey);

			ptrModule->lookup<ComponentWrapper<Component_Create>>().TestFunc();

			::pb::rpc::RPC_LoginPendingRequest rpcRequest;
			rpcRequest.set_account_id(iId);
			rpcRequest.set_session_key(sessionKey);
			rpcRequest.set_db_id(1);
			rpcRequest.set_version(0);

			::rpc_msg::CHANNEL server;
			server.set_realm(apie::Ctx::getThisChannel().realm());
			server.set_type(gatewayOpt.value().type());
			server.set_id(gatewayOpt.value().id());

			auto rpcCB = [info, response](const apie::status::Status& status, const std::shared_ptr<::pb::rpc::RPC_LoginPendingResponse>& responsePtr) mutable {
				if (!status.ok())
				{
					response->set_error_code(apie::toUnderlyingType(status.code()));
					service::ServiceManager::sendResponse(info, *response);
					return;
				}

				service::ServiceManager::sendResponse(info, *response);
			};
			apie::rpc::RPC_Call<::pb::rpc::RPC_LoginPendingRequest, ::pb::rpc::RPC_LoginPendingResponse>(server, ::pb::rpc::OP_RPC_LoginPending, rpcRequest, rpcCB);
		}
		else
		{
			response->set_error_code(pb::core::CONTROL_ERROR);

			service::ServiceManager::sendResponse(info, *response);
		}
	};
	AccountLoader::LoadFromDb(iId, doneCb);

	return apie::status::E_ReturnType::kRT_Async;
}


}

