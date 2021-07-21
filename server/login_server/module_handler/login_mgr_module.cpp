#include "login_mgr_module.h"

#include "../../common/dao/model_account.h"
#include "../../common/dao/model_account_name.h"
#include "../../common/opcodes.h"


namespace apie {

void LoginMgrModule::init()
{
	// PUBSUB
	auto& pubsub = apie::pubsub::PubSubManagerSingleton::get();
	pubsub.subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, LoginMgrModule::PubSub_logicCmd);
	pubsub.subscribe<::pubsub::SERVER_PEER_CLOSE>(::pubsub::PT_ServerPeerClose, LoginMgrModule::PubSub_serverPeerClose);


	// CMD
	auto& cmd = LogicCmdHandlerSingleton::get();
	cmd.init();
	cmd.registerOnCmd("nats_publish", "nats_publish", LoginMgrModule::Cmd_natsPublish);

	return;
}


void LoginMgrModule::ready()
{
	// CLIENT OPCODE
	auto& server = apie::service::ServiceHandlerSingleton::get().server;
	server.createService<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>(::apie::OP_MSG_REQUEST_ACCOUNT_LOGIN_L, LoginMgrModule::handleAccountNotify);

	return;
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

apie::status::Status LoginMgrModule::handleAccount(uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_ACCOUNT_LOGIN_L>& response)
{
	std::stringstream ss;
	ss << "handleAccount:" << request->ShortDebugString();
	ASYNC_PIE_LOG("LoginMgr/handleAccount", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void LoginMgrModule::handleAccountNotify(uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>& notify)
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

	rpc_msg::MSG_RPC_REQUEST_ECHO params;
	params.set_value1(200);
	params.set_value2("test_hello");

	::rpc_msg::CHANNEL server;
	server.set_realm(realm);
	server.set_type(type);
	server.set_id(id);

	//auto rpcObj = apie::rpc::RPCClientManagerSingleton::get().createRPCClient<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(server, rpc_msg::RPC_EchoTest, nullptr);
	//rpcObj->sendRequest(params);

	auto cb = [](const apie::status::Status& status, const std::shared_ptr<rpc_msg::MSG_RPC_RESPONSE_ECHO>& response) {
		if (!status.ok())
		{
			return;
		}

		std::stringstream ss;
		ss << "RPC_echoCb:" << response->ShortDebugString();
	};
	apie::rpc::RPC_Call<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(server, rpc_msg::RPC_EchoTest, params, cb);

}

}

