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
		PIE_LOG("Cmd_client/Cmd_client", PIE_CYCLE_DAY, PIE_WARNING, "unregister cmd : %s", msg->ShortDebugString().c_str());
		return;
	}

	try
	{
		handlerOpt.value()(*msg);
	}
	catch (std::exception& e)
	{
		std::stringstream ss;
		ss << msg->ShortDebugString();
		PIE_LOG("Exception/Exception", PIE_CYCLE_DAY, PIE_ERROR, "handle cmd error: %s", ss.str().c_str());
	}
}

/*
* 输入格式:  client|IggId|module_name|cmd|params...
* 转换到LOGIC_CMD结构:  cmd:client, params:IggId|module_name|cmd|params...	
*/
void TestServerModule::Cmd_client(::pubsub::LOGIC_CMD& cmd)
{
	if (cmd.params_size() < 2)
	{
		PIE_LOG("Cmd_client/Cmd_client", PIE_CYCLE_DAY, PIE_WARNING, "Expected >= %d, got: %d", 2, cmd.params_size());
		return;
	}

	//没有模块命令的，以模块名作为命令
	std::set<std::string> noModuleCmd;
	noModuleCmd.insert("login");
	noModuleCmd.insert("logout");

	auto iModuleIndex = 1;
	if (noModuleCmd.count(cmd.params()[iModuleIndex]) != 0)
	{
		auto ptrAdd = cmd.add_params();
		*ptrAdd = cmd.params()[iModuleIndex];
	}

	if (cmd.params_size() < 3)
	{
		PIE_LOG("Cmd_client/Cmd_client", PIE_CYCLE_DAY, PIE_WARNING, "Expected >= %d, got: %d", 3, cmd.params_size());
		return;
	}

	::pubsub::TEST_CMD newMsg;

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

	newMsg.set_module_name(cmd.params()[1]);
	newMsg.set_cmd(cmd.params()[2]);
	for (int i = 3; i < cmd.params().size(); i++)
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

/*
* 输入格式:  auto_test
* 转换到LOGIC_CMD结构:  cmd:auto_test, params:
*/
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

