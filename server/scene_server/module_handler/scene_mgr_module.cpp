#include "module_handler/scene_mgr_module.h"

#include "../../common/opcodes.h"


namespace apie {

void SceneMgrModule::init()
{
	// PUBSUB
	auto& pubsub = apie::pubsub::PubSubManagerSingleton::get();
	pubsub.subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, SceneMgrModule::PubSub_logicCmd);

	// CMD
	auto& cmd = LogicCmdHandlerSingleton::get();
	cmd.init();
}

void SceneMgrModule::ready()
{
	// RPC
	auto& rpc = apie::rpc::RPCServerManagerSingleton::get();
	rpc.createRPCServer<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(rpc_msg::RPC_EchoTest, SceneMgrModule::RPC_echo);


	// FORWARD
	auto& mux = apie::forward::ForwardManagerSingleton::get();
	mux.createService<::login_msg::MSG_REQUEST_ECHO, ::apie::OP_MSG_RESPONSE_ECHO, ::login_msg::MSG_RESPONSE_ECHO>(::apie::OP_MSG_REQUEST_ECHO, SceneMgrModule::Forward_echo);
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
	PIE_LOG("SceneMgr/Forward_handlEcho", PIE_CYCLE_DAY, PIE_NOTICE, "%s", request->DebugString().c_str());

	response->set_value1(request->value1());
	response->set_value2(request->value2() + "|response");

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status SceneMgrModule::RPC_echo(const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<rpc_msg::MSG_RPC_REQUEST_ECHO>& request, std::shared_ptr<rpc_msg::MSG_RPC_RESPONSE_ECHO>& response)
{
	PIE_LOG("SceneMgr/RPC_echo", PIE_CYCLE_DAY, PIE_NOTICE, "%s", request->DebugString().c_str());

	response->set_value1(100);
	response->set_value2("hello world");
	return { apie::status::StatusCode::OK, "" };
}


}

