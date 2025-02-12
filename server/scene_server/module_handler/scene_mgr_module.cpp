#include "module_handler/scene_mgr_module.h"



namespace apie {

void SceneMgrModule::init()
{
	// PUBSUB
	auto& pubsub = apie::pubsub::PubSubManagerSingleton::get();
	pubsub.subscribe<::pubsub::LOGIC_CMD>(::pubsub::PT_LogicCmd, SceneMgrModule::PubSub_logicCmd);

	// CMD
	auto& cmd = LogicCmdHandlerSingleton::get();
	cmd.init();
	cmd.registerOnCmd("nats_publish", "nats_publish", SceneMgrModule::Cmd_natsPublish);
}

void SceneMgrModule::ready()
{
	// RPC
	using namespace ::pb::rpc;
	INTRA_REGISTER_RPC(EchoTest, SceneMgrModule::RPC_echoTest);


	// FORWARD
	using namespace ::login_msg;
	REGISTER_FORWARD_REQUEST(Echo, SceneMgrModule::Forward_echo);
}

void SceneMgrModule::PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{
	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

apie::status::E_ReturnType SceneMgrModule::Forward_echo(const ::rpc_msg::RoleIdentifier& role, const std::shared_ptr<::login_msg::EchoRequest>& request, std::shared_ptr<::login_msg::EchoResponse>& response)
{
	PIE_LOG(PIE_NOTICE, "{}", request->DebugString().c_str());

	response->set_value1(request->value1());
	response->set_value2(request->value2() + "|response");

	return apie::status::E_ReturnType::kRT_Sync;

	//apie::forward::ForwardManager::sendNotifyToGW(role.user_id(), role.info().response_opcode(), *response);

	//apie::forward::ForwardManager::sendResponse(role, *response);
	//return { apie::status::StatusCode::OK_ASYNC, "" };
}

apie::status::Status SceneMgrModule::RPC_echoTest(const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<pb::rpc::RPC_EchoTestRequest>& request, std::shared_ptr<pb::rpc::RPC_EchoTestResponse>& response)
{
	PIE_LOG(PIE_NOTICE, "{}", request->DebugString().c_str());

	response->set_value1(request->value1());
	response->set_value2(request->value2() + " scene");

	return { apie::status::StatusCode::OK, "" };
}

void SceneMgrModule::Cmd_natsPublish(::pubsub::LOGIC_CMD& cmd)
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
	server.set_actor_id("1");

	//auto rpcObj = apie::rpc::createRPCClient<rpc_msg::RPC_EchoTestRequest, rpc_msg::RPC_EchoTestResponse>(server, rpc_msg::OP_RPC_EchoTest, nullptr);
	//rpcObj->sendRequest(params);

	auto cb = [](const apie::status::Status& status, const std::shared_ptr<pb::rpc::RPC_EchoTestResponse>& response) {
		if (!status.ok())
		{
			return;
		}

		std::stringstream ss;
		ss << "RPC_echoCb:" << response->ShortDebugString() << std::endl;
		ASYNC_PIE_LOG(PIE_NOTICE, "{}", ss.str());
		};
	apie::rpc::RPC_Call<pb::rpc::RPC_EchoTestRequest, pb::rpc::RPC_EchoTestResponse>(server, pb::rpc::OP_RPC_EchoTest, params, cb);
}

}

