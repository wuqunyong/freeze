#include "test_server.h"

#include "json/json.h"

#include "../../apie/redis_driver/redis_client.h"
#include "../../common/opcodes.h"


#include "test_runner.h"

namespace apie {


apie::status::Status TestServerMgr::init()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_Test_Client });
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}


	// PUBSUB
	apie::pubsub::PubSubManagerSingleton::get().subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, TestServerMgr::PubSub_logicCmd);
	
	// CMD
	auto& cmd = LogicCmdHandlerSingleton::get();
	cmd.init();
	cmd.registerOnCmd("client", "client", TestServerMgr::Cmd_client);
	cmd.registerOnCmd("auto_test", "auto_test", TestServerMgr::Cmd_autoTest);

	// OPCODE
	apie::service::ServiceHandlerSingleton::get().client.setDefaultFunc(TestServerMgr::handleDefaultOpcodes);

	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_ACCOUNT_LOGIN_L, "login_msg.MSG_RESPONSE_ACCOUNT_LOGIN_L");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_CLIENT_LOGIN, "login_msg.MSG_RESPONSE_CLIENT_LOGIN");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_ECHO, "login_msg.MSG_RESPONSE_ECHO");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_HANDSHAKE_INIT, "login_msg.MSG_RESPONSE_HANDSHAKE_INIT");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_HANDSHAKE_ESTABLISHED, "login_msg.MSG_RESPONSE_HANDSHAKE_ESTABLISHED");


	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestServerMgr::start()
{
	std::string ip = apie::CtxSingleton::get().getConfigs()->clients.socket_address.address;
	uint16_t port = apie::CtxSingleton::get().getConfigs()->clients.socket_address.port_value;
	uint16_t type = apie::CtxSingleton::get().getConfigs()->clients.socket_address.type;
	uint32_t maskFlag = apie::CtxSingleton::get().getConfigs()->clients.socket_address.mask_flag;

	m_ptrClientProxy = apie::ClientProxy::createClientProxy();
	auto connectCb = [](apie::ClientProxy* ptrClient, uint32_t iResult) {
		if (iResult == 0)
		{
			apie::hook::HookRegistrySingleton::get().triggerHook(hook::HookPoint::HP_Ready);
		}
		return true;
	};
	m_ptrClientProxy->connect(ip, port, static_cast<apie::ProtocolType>(type), maskFlag, connectCb);
	m_ptrClientProxy->addReconnectTimer(60000);

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestServerMgr::ready()
{
	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ServerStatus", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void TestServerMgr::exit()
{
}

void TestServerMgr::addMockRole(std::shared_ptr<MockRole> ptrMockRole)
{
	auto iRoleId = ptrMockRole->getRoleId();
	m_mockRole[iRoleId] = ptrMockRole;
}

std::shared_ptr<MockRole> TestServerMgr::findMockRole(uint64_t iRoleId)
{
	auto findIte = m_mockRole.find(iRoleId);
	if (findIte == m_mockRole.end())
	{
		return nullptr;
	}

	return findIte->second;
}

void TestServerMgr::removeMockRole(uint64_t iRoleId)
{
	m_mockRole.erase(iRoleId);
}

void TestServerMgr::addSerialNumRole(uint64_t iSerialNum, uint64_t iRoleId)
{
	m_serialNumRole[iSerialNum] = iRoleId;
}

std::optional<uint64_t> TestServerMgr::findRoleIdBySerialNum(uint64_t iSerialNum)
{
	auto findIte = m_serialNumRole.find(iSerialNum);
	if (findIte == m_serialNumRole.end())
	{
		return std::nullopt;
	}

	return findIte->second;
}

void TestServerMgr::removeSerialNum(uint64_t iSerialNum)
{
	m_serialNumRole.erase(iSerialNum);
}


void TestServerMgr::PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{
	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

void TestServerMgr::Cmd_client(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 2)
	{
		std::cout << "invalid params" << std::endl;
		return;
	}

	uint64_t iRoleId = std::stoull(cmd.params()[0]);
	auto ptrMockRole = TestServerMgrSingleton::get().findMockRole(iRoleId);
	if (ptrMockRole == nullptr)
	{
		if (cmd.params()[1] == "account_login")
		{
			ptrMockRole = MockRole::createMockRole(iRoleId);
			ptrMockRole->start();

			TestServerMgrSingleton::get().addMockRole(ptrMockRole);
		}
		else
		{
			std::cout << "not login|account:" << iRoleId << std::endl;
			return;
		}
	}

	::pubsub::LOGIC_CMD newMsg;
	for (int i = 1; i < cmd.params().size(); i++)
	{
		if (i == 1)
		{
			newMsg.set_cmd(cmd.params()[i]);
		}
		else
		{
			auto ptrAdd = newMsg.add_params();
			*ptrAdd = cmd.params()[i];
		}
	}
	ptrMockRole->pushMsg(newMsg);
}

void TestServerMgr::Cmd_autoTest(::pubsub::LOGIC_CMD& cmd)
{
	std::cout << "start auto_test" << std::endl;

	TestRunnerMgrSingleton::get().init();
	TestRunnerMgrSingleton::get().run();
}

void TestServerMgr::handleDefaultOpcodes(uint64_t serialNum, uint32_t opcodes, const std::string& msg)
{
	auto iRoleIdOpt = TestServerMgrSingleton::get().findRoleIdBySerialNum(serialNum);
	if (!iRoleIdOpt.has_value())
	{
		return;
	}

	uint64_t iRoleId = iRoleIdOpt.value();
	auto ptrMockRole = TestServerMgrSingleton::get().findMockRole(iRoleId);
	if (ptrMockRole == nullptr)
	{
		return;
	}

	ptrMockRole->handleResponse(serialNum, opcodes, msg);
	ptrMockRole->handlePendingResponse(serialNum, opcodes, msg);
	ptrMockRole->handlePendingNotify(serialNum, opcodes, msg);
}

}

