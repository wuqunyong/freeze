#include "login_mgr.h"

#include "../../apie/common/string_utils.h"
#include "../../common/dao/model_account.h"
#include "../../common/dao/model_account_name.h"
#include "../../common/opcodes.h"


namespace APie {

std::tuple<uint32_t, std::string> LoginMgr::init()
{
	auto bResult = APie::CtxSingleton::get().checkIsValidServerType({ common::EPT_Login_Server });
	if (!bResult)
	{
		return std::make_tuple(Hook::HookResult::HR_Error, "invalid Type");
	}

	// CMD
	apie::pubsub::PubSubManagerSingleton::get().subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, LoginMgr::onLogicCommnad);

	LogicCmdHandlerSingleton::get().init();
	LogicCmdHandlerSingleton::get().registerOnCmd("nats_publish", "nats_publish", LoginMgr::onNatsPublish);

	// RPC
	apie::rpc::rpcInit();

	return std::make_tuple(Hook::HookResult::HR_Ok, "");
}

std::tuple<uint32_t, std::string> LoginMgr::start()
{
	APie::Hook::HookRegistrySingleton::get().triggerHook(Hook::HookPoint::HP_Ready);
	return std::make_tuple(Hook::HookResult::HR_Ok, "HR_Ok");
}

std::tuple<uint32_t, std::string> LoginMgr::ready()
{
	// PubSub
	apie::pubsub::PubSubManagerSingleton::get().subscribe<::pubsub::SERVER_PEER_CLOSE>(::pubsub::PT_ServerPeerClose, LoginMgr::onServerPeerClose);


	// CLIENT OPCODE
	auto& server = apie::service::ServiceHandlerSingleton::get().server;
	server.createService<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>(::APie::OP_MSG_REQUEST_ACCOUNT_LOGIN_L, LoginMgr::handleAccountNotify);

	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ServerStatus", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return std::make_tuple(Hook::HookResult::HR_Ok, "");
}

void LoginMgr::exit()
{

}

void LoginMgr::onLogicCommnad(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{
	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

void LoginMgr::onServerPeerClose(const std::shared_ptr<::pubsub::SERVER_PEER_CLOSE>& msg)
{
	std::stringstream ss;

	ss << "topic:" << ",refMsg:" << msg->ShortDebugString();
	ASYNC_PIE_LOG("LoginMgr/onServerPeerClose", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	//uint64_t iSerialNum = refMsg.serial_num();

}
apie::status::Status LoginMgr::handleAccount(uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>& request, std::shared_ptr<::login_msg::MSG_RESPONSE_ACCOUNT_LOGIN_L>& response)
{
	std::stringstream ss;
	ss << "handleAccount:" << request->ShortDebugString();
	ASYNC_PIE_LOG("LoginMgr/handleAccount", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void LoginMgr::handleAccountNotify(uint64_t iSerialNum, const std::shared_ptr<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>& notify)
{
	std::stringstream ss;
	ss << "handleAccount:" << notify->ShortDebugString();
	ASYNC_PIE_LOG("LoginMgr/handleAccount", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());
}

void LoginMgr::onNatsPublish(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 5)
	{
		return;
	}

	uint32_t realm = std::stoul(cmd.params()[0]);
	uint32_t type = std::stoul(cmd.params()[1]);
	uint32_t id = std::stoul(cmd.params()[2]);

	std::string channel = APie::Event::NatsManager::GetTopicChannel(realm, type, id);

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

	apie::rpc::RPC_Call<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(server, rpc_msg::RPC_EchoTest, params, LoginMgr::RPC_echoCb);

}

void LoginMgr::RPC_echoCb(const apie::status::Status& status, const std::shared_ptr<rpc_msg::MSG_RPC_RESPONSE_ECHO>& response)
{
	if (!status.ok())
	{
		return;
	}

	std::stringstream ss;
	ss << "RPC_echoCb:" << response->ShortDebugString();
}

}

