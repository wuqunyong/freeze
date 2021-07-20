#include "scene_mgr.h"

#include "../../common/opcodes.h"


namespace apie {

apie::status::Status SceneMgr::init()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_Scene_Server });
	if (!bResult)
	{
		return {apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}

	// CMD
	LogicCmdHandlerSingleton::get().init();
	apie::pubsub::PubSubManagerSingleton::get().subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, SceneMgr::onLogicCommnad);


	// RPC


	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status SceneMgr::start()
{
	apie::hook::HookRegistrySingleton::get().triggerHook(hook::HookPoint::HP_Ready);

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status SceneMgr::ready()
{
	// CLIENT OPCODE
	//auto& forwardHandler = APie::Api::ForwardHandlerSingleton::get();
	//forwardHandler.server.bind(::APie::OP_MSG_REQUEST_ECHO, SceneMgr::Forward_handlEcho);

	apie::rpc::RPCServerManagerSingleton::get().createRPCServer<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(rpc_msg::RPC_EchoTest, SceneMgr::RPC_echo);


	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ServerStatus", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void SceneMgr::exit()
{

}

void SceneMgr::onLogicCommnad(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{

	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

void SceneMgr::Forward_handlEcho(::rpc_msg::RoleIdentifier roleIdentifier, ::login_msg::MSG_REQUEST_ECHO request)
{
	PIE_LOG("SceneMgr/Forward_handlEcho", PIE_CYCLE_DAY, PIE_NOTICE, "%s", request.DebugString().c_str());

	//uint64_t iCurMS = Ctx::getCurMilliseconds();

	::login_msg::MSG_RESPONSE_ECHO response;
	response.set_value1(request.value1());
	response.set_value2(request.value2() + "|response");
}

apie::status::Status SceneMgr::RPC_echo(const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<rpc_msg::MSG_RPC_REQUEST_ECHO>& request, std::shared_ptr<rpc_msg::MSG_RPC_RESPONSE_ECHO>& response)
{
	PIE_LOG("SceneMgr/RPC_echo", PIE_CYCLE_DAY, PIE_NOTICE, "%s", request->DebugString().c_str());

	response->set_value1(100);
	response->set_value2("hello world");
	return { apie::status::StatusCode::OK, "" };
}


}

