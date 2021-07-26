#include "test_server_module.h"

#include "json/json.h"

#include "../../apie/redis_driver/redis_client.h"
#include "../../common/opcodes.h"


#include "../logic/test_runner.h"
#include "../logic/test_server.h"

namespace apie {


void TestServerModule::init()
{
	// PUBSUB
	apie::pubsub::PubSubManagerSingleton::get().subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, TestServerModule::PubSub_logicCmd);
	
	// CMD
	auto& cmd = LogicCmdHandlerSingleton::get();
	cmd.init();
	cmd.registerOnCmd("client", "client", TestServerModule::Cmd_client);
	cmd.registerOnCmd("auto_test", "auto_test", TestServerModule::Cmd_autoTest);

	// OPCODE
	apie::service::ServiceHandlerSingleton::get().client.setDefaultFunc(TestServerModule::handleDefaultOpcodes);

	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_ACCOUNT_LOGIN_L, "login_msg.MSG_RESPONSE_ACCOUNT_LOGIN_L");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_CLIENT_LOGIN, "login_msg.MSG_RESPONSE_CLIENT_LOGIN");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_ECHO, "login_msg.MSG_RESPONSE_ECHO");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_HANDSHAKE_INIT, "login_msg.MSG_RESPONSE_HANDSHAKE_INIT");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_HANDSHAKE_ESTABLISHED, "login_msg.MSG_RESPONSE_HANDSHAKE_ESTABLISHED");

}


void TestServerModule::ready()
{
}

void TestServerModule::PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{
	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

void TestServerModule::Cmd_client(::pubsub::LOGIC_CMD& cmd)
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

void TestServerModule::Cmd_autoTest(::pubsub::LOGIC_CMD& cmd)
{
	std::cout << "start auto_test" << std::endl;

	TestRunnerMgrSingleton::get().init();
	TestRunnerMgrSingleton::get().run();
}

void TestServerModule::handleDefaultOpcodes(uint64_t serialNum, uint32_t opcodes, const std::string& msg)
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

