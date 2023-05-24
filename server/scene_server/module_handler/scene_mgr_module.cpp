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

	response->set_value1(100);
	response->set_value2("hello world");
	return { apie::status::StatusCode::OK, "" };
}


}

