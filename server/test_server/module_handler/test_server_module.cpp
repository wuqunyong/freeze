#include "module_handler/test_server_module.h"

#include "../../common/opcodes.h"

#include "logic/test_runner.h"
#include "logic/test_server.h"

namespace apie {


void TestServerModule::init()
{
}

void TestServerModule::ready()
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

	//MockRole::registerPbOpcodeName(MergeOpcode(_MSG_GAMESERVER_LOGINRESP, 0), "pb.login.LoginLS_Resp");
	for (const auto& elems : apie::CtxSingleton::get().getConfigs()->pb_map_vec)
	{
		MockRole::registerPbOpcodeName(MergeOpcode(elems.type, elems.cmd), elems.pb_name);
	}
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
	auto ptrMockRole = APieGetModule<apie::TestServerMgr>()->findMockRole(iRoleId);
	if (ptrMockRole == nullptr)
	{
		if (cmd.params()[1] == "login")
		{
			ptrMockRole = MockRole::createMockRole(iRoleId);
			ptrMockRole->start();

			APieGetModule<apie::TestServerMgr>()->addMockRole(ptrMockRole);
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

void TestServerModule::handleDefaultOpcodes(MessageInfo info, const std::string& msg)
{
	auto iRoleIdOpt = APieGetModule<apie::TestServerMgr>()->findRoleIdBySerialNum(info.iSessionId);
	if (!iRoleIdOpt.has_value())
	{
		return;
	}

	uint64_t iRoleId = iRoleIdOpt.value();
	auto ptrMockRole = APieGetModule<apie::TestServerMgr>()->findMockRole(iRoleId);
	if (ptrMockRole == nullptr)
	{
		return;
	}

	ptrMockRole->handleResponse(info, msg);
	ptrMockRole->handlePendingResponse(info, msg);
	ptrMockRole->handlePendingNotify(info, msg);
}


}

