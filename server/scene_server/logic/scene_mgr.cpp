#include "scene_mgr.h"

#include "../../pb_msg/core/opcodes.pb.h"
#include "../../common/opcodes.h"


namespace APie {

std::tuple<uint32_t, std::string> SceneMgr::init()
{
	auto bResult = APie::CtxSingleton::get().checkIsValidServerType({ common::EPT_Scene_Server });
	if (!bResult)
	{
		return std::make_tuple(Hook::HookResult::HR_Error, "invalid Type");
	}

	// CMD
	LogicCmdHandlerSingleton::get().init();
	APie::PubSubSingleton::get().subscribe(::pubsub::PUB_TOPIC::PT_LogicCmd, SceneMgr::onLogicCommnad);


	// RPC
	apie::rpc::rpcInit();


	return std::make_tuple(Hook::HookResult::HR_Ok, "");
}

std::tuple<uint32_t, std::string> SceneMgr::start()
{
	APie::Hook::HookRegistrySingleton::get().triggerHook(Hook::HookPoint::HP_Ready);

	return std::make_tuple(Hook::HookResult::HR_Ok, "");
}

std::tuple<uint32_t, std::string> SceneMgr::ready()
{
	// CLIENT OPCODE
	auto& forwardHandler = APie::Api::ForwardHandlerSingleton::get();
	forwardHandler.server.bind(::APie::OP_MSG_REQUEST_ECHO, SceneMgr::Forward_handlEcho);

	apie::rpc::RPCServerManagerSingleton::get().createRPCServer<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(rpc_msg::RPC_EchoTest, SceneMgr::RPC_echo);


	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ServerStatus", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return std::make_tuple(Hook::HookResult::HR_Ok, "");
}

void SceneMgr::exit()
{

}

void SceneMgr::onLogicCommnad(uint64_t topic, ::google::protobuf::Message& msg)
{

	auto& command = dynamic_cast<::pubsub::LOGIC_CMD&>(msg);
	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(command.cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(command);
}

void SceneMgr::Forward_handlEcho(::rpc_msg::RoleIdentifier roleIdentifier, ::login_msg::MSG_REQUEST_ECHO request)
{
	PIE_LOG("SceneMgr/Forward_handlEcho", PIE_CYCLE_DAY, PIE_NOTICE, "%s", request.DebugString().c_str());

	//uint64_t iCurMS = Ctx::getCurMilliseconds();

	::login_msg::MSG_RESPONSE_ECHO response;
	response.set_value1(request.value1());
	response.set_value2(request.value2() + "|response");
	Network::OutputStream::sendMsgToUserByGateway(roleIdentifier, APie::OP_MSG_RESPONSE_ECHO, response);
}

apie::status::Status SceneMgr::RPC_echo(const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<rpc_msg::MSG_RPC_REQUEST_ECHO>& request, std::shared_ptr<rpc_msg::MSG_RPC_RESPONSE_ECHO>& response)
{
	PIE_LOG("SceneMgr/RPC_echo", PIE_CYCLE_DAY, PIE_NOTICE, "%s", request->DebugString().c_str());

	response->set_value1(100);
	response->set_value2("hello world");
	return { apie::status::StatusCode::OK, "" };
}


}

