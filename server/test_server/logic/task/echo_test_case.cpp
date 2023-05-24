#include "logic/task/echo_test_case.h"


namespace apie {
	using namespace std::placeholders;

	EchoTestCase::EchoTestCase(MockRole& role, uint32_t type) :
		TestCase(role, type)
	{

	}

	void EchoTestCase::setUp()
	{
		auto iRPCId = sendEcho();
		auto bindCb = std::bind(&EchoTestCase::RPC_onEcho, this, _1, _2, _3);
		this->getRole().waitRPC(iRPCId, pb::core::OP_EchoRequest, bindCb);
	}

	void EchoTestCase::tearDown()
	{
		m_iIndex = 0;
	}

	std::shared_ptr<TestCase> EchoTestCase::createMethod(MockRole& role, uint32_t type)
	{
		return std::make_shared<EchoTestCase>(role, type);
	}

	uint32_t EchoTestCase::getFactoryType()
	{
		return ETCT_Echo;
	}

	uint32_t EchoTestCase::sendEcho()
	{
		m_iIndex++;

		::login_msg::EchoRequest request;
		request.set_value1(m_iIndex);
		request.set_value2("hello world");
		return this->getRole().sendMsg(pb::core::OP_EchoRequest, request);
	}

	void EchoTestCase::RPC_onEcho(MockRole* ptrRole, MessageInfo info, const std::string& msg)
	{
		::login_msg::EchoResponse response;
		bool bResult = response.ParseFromString(msg);
		if (!bResult)
		{
			this->setStatus(ETestCaseStatus::ECS_FAILURE);
			return;
		}

		this->setStatus(ETestCaseStatus::ECS_SUCCESS);
	}

}


