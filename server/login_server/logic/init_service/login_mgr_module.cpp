#include "logic/init_service/login_mgr_module.h"

#include "../../../common/dao/init.h"

#include "logic/init_service/account.h"
#include "logic/init_service/componet_create.h"
#include "logic/init_service/componet_name.h"

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
}


void LoginMgrModule::ready()
{
	// CLIENT OPCODE
	//S_REGISTER_REQUEST(ACCOUNT_LOGIN_L, LoginMgrModule::handleAccount);
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
			ptrLoader->lookup<ComponentWrapper<Module_Create>>().TestFunc();
		}
	};
	AccountLoader::LoadFromDb(iId, doneCb);
}

}

