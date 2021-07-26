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


	apie::pubsub::PubSubManagerSingleton::get().subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, TestServerMgr::PubSub_logicCmd);
	
	apie::service::ServiceHandlerSingleton::get().client.setDefaultFunc(TestServerMgr::handleDefaultOpcodes);

	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_ACCOUNT_LOGIN_L, "login_msg.MSG_RESPONSE_ACCOUNT_LOGIN_L");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_CLIENT_LOGIN, "login_msg.MSG_RESPONSE_CLIENT_LOGIN");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_ECHO, "login_msg.MSG_RESPONSE_ECHO");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_HANDSHAKE_INIT, "login_msg.MSG_RESPONSE_HANDSHAKE_INIT");
	MockRole::registerPbOpcodeName(OP_MSG_RESPONSE_HANDSHAKE_ESTABLISHED, "login_msg.MSG_RESPONSE_HANDSHAKE_ESTABLISHED");

	//Api::OpcodeHandlerSingleton::get().client.bind(::apie::OP_MSG_RESPONSE_CLIENT_LOGIN, TestServerMgr::handleResponseClientLogin, ::login_msg::MSG_RESPONSE_CLIENT_LOGIN::default_instance());
	//Api::OpcodeHandlerSingleton::get().client.bind(::apie::OP_MSG_RESPONSE_ECHO, TestServerMgr::handleResponseEcho, ::login_msg::MSG_RESPONSE_ECHO::default_instance());


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
	auto& command = *msg;

	if (command.cmd() == "login")
	{
		if (command.params_size() < 2)
		{
			return;
		}

		::login_msg::MSG_REQUEST_CLIENT_LOGIN request;
		request.set_user_id(std::stoull(command.params()[0]));
		request.set_version(std::stoi(command.params()[1]));
		request.set_session_key(command.params()[2]);

		TestServerMgrSingleton::get().m_ptrClientProxy->sendMsg(::apie::OP_MSG_REQUEST_CLIENT_LOGIN, request);

		std::cout << "send|iSerialNum:" << TestServerMgrSingleton::get().m_ptrClientProxy->getSerialNum() << "|request:" << request.ShortDebugString() << std::endl;
	}
	else if (command.cmd() == "echo")
	{
		if (command.params_size() < 1)
		{
			return;
		}

		uint64_t iCurMS = Ctx::getCurMilliseconds();

		::login_msg::MSG_REQUEST_ECHO request;
		request.set_value1(iCurMS);
		request.set_value2(command.params()[0]);

		TestServerMgrSingleton::get().m_ptrClientProxy->sendMsg(::apie::OP_MSG_REQUEST_ECHO, request);

		std::cout << "send|iSerialNum:" << TestServerMgrSingleton::get().m_ptrClientProxy->getSerialNum() << "|request:" << request.ShortDebugString() << std::endl;
	}
	else if (command.cmd() == "redis")
	{
		if (command.params_size() < 2)
		{
			return;
		}


		auto key = std::make_tuple(1, 1);
		std::string sTable = command.params()[0];  //command.params()[0];
		std::string field = command.params()[1];


		if (sTable == "hs_player_summary")
		{
			auto redisClient = RedisClientFactorySingleton::get().getClient(key);
			if (redisClient == nullptr)
			{
				return;
			}

			if (!redisClient->client().is_connected())
			{
				return;
			}

			auto cb = [sTable](cpp_redis::reply &reply) {
				if (reply.is_error())
				{
					return;
				}


				if (reply.is_null())
				{
					return;
				}

				if (!reply.is_bulk_string())
				{
					return;
				}

				::role_msg::ROLE_SUMMARY summary;

				std::string content = reply.as_string();
				if (!summary.ParseFromString(content))
				{
					return;
				}

				std::cout << summary.DebugString() << std::endl;
			};
			redisClient->client().hget(sTable, field, cb);
			redisClient->client().commit();
		}
		else if (sTable == "order_info")
		{
			field = command.params()[1] + "|" + command.params()[2];

			auto redisClient = RedisClientFactorySingleton::get().getClient(key);
			if (redisClient == nullptr)
			{
				return;
			}

			if (!redisClient->client().is_connected())
			{
				return;
			}

			auto cb = [sTable](cpp_redis::reply &reply) {
				if (reply.is_error())
				{
					return;
				}


				if (reply.is_null())
				{
					return;
				}

				if (!reply.is_bulk_string())
				{
					return;
				}

				//::gm_msg::ORDER_INFO order;

				//std::string content = reply.as_string();
				//if (!order.ParseFromString(content))
				//{
				//	return;
				//}

				//std::cout << order.DebugString() << std::endl;
			};
			redisClient->client().hget(sTable, field, cb);
			redisClient->client().commit();
		}
		else
		{

		}

	}
	else if (command.cmd() == "client")
	{
		if (command.params_size() < 2)
		{
			std::cout << "invalid params" << std::endl;
			return;
		}

		uint64_t iRoleId = std::stoull(command.params()[0]);
		auto ptrMockRole = TestServerMgrSingleton::get().findMockRole(iRoleId);
		if (ptrMockRole == nullptr)
		{
			if (command.params()[1] == "account_login")
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
		for (int i = 1; i < command.params().size(); i++)
		{
			if (i == 1)
			{
				newMsg.set_cmd(command.params()[i]);
			}
			else
			{
				auto ptrAdd = newMsg.add_params();
				*ptrAdd = command.params()[i];
			}
		}
		ptrMockRole->pushMsg(newMsg);
	}
	else if (command.cmd() == "auto_test")
	{
		std::cout << "start auto_test" << std::endl;

		TestRunnerMgrSingleton::get().init();
		TestRunnerMgrSingleton::get().run();
	}

}

//void TestServerMgr::handleResponseClientLogin(uint64_t iSerialNum, const ::login_msg::MSG_RESPONSE_CLIENT_LOGIN& response)
//{
//	std::cout << "recv|iSerialNum:" << iSerialNum << "|response:" << response.ShortDebugString() << std::endl;
//}
//
//void TestServerMgr::handleResponseEcho(uint64_t iSerialNum, const ::login_msg::MSG_RESPONSE_ECHO& response)
//{
//	std::cout << "recv|iSerialNum:" << iSerialNum << "|response:" << response.ShortDebugString() << std::endl;
//}

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

