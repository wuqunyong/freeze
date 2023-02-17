#include "module_handler/scene_mgr_module.h"

#include "../../common/opcodes.h"


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
	using namespace ::rpc_msg;
	INTRA_REGISTER_RPC(EchoTest, SceneMgrModule::RPC_echoTest);


	// FORWARD
	using namespace ::login_msg;
	S_REGISTER_FORWARD_REQUEST(ECHO, SceneMgrModule::Forward_echo);
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

apie::status::Status SceneMgrModule::Forward_echo(const ::rpc_msg::RoleIdentifier& role, const std::shared_ptr<::login_msg::MSG_REQUEST_ECHO>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_ECHO>& response)
{
	PIE_FMT_LOG("SceneMgr/Forward_handlEcho", PIE_CYCLE_DAY, PIE_NOTICE, "{}", request->DebugString().c_str());

	response->set_value1(request->value1());
	response->set_value2(request->value2() + "|response");

	return { apie::status::StatusCode::OK, "" };

	//apie::forward::ForwardManager::sendNotifyToGW(role.user_id(), role.info().response_opcode(), *response);

	//apie::forward::ForwardManager::sendResponse(role, *response);
	//return { apie::status::StatusCode::OK_ASYNC, "" };
}

apie::status::Status SceneMgrModule::RPC_echoTest(const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<rpc_msg::RPC_EchoTestRequest>& request, std::shared_ptr<rpc_msg::RPC_EchoTestResponse>& response)
{
	PIE_FMT_LOG("SceneMgr/RPC_echo", PIE_CYCLE_DAY, PIE_NOTICE, "{}", request->DebugString().c_str());

	response->set_value1(100);
	response->set_value2("hello world");
	return { apie::status::StatusCode::OK, "" };
}


}

