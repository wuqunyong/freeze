#include "logic/task/login_test_case.h"


namespace apie {
	using namespace std::placeholders;

	LoginTestCase::LoginTestCase(MockRole& role, uint32_t type) :
		TestCase(role, type)
	{

	}

	void LoginTestCase::setUp()
	{
		::pubsub::TEST_CMD newMsg;
		newMsg.set_module_name("login");
		newMsg.set_cmd("login");

		this->getRole().pushMsg(newMsg);

		auto bindCb = std::bind(&LoginTestCase::handleLogin, this, _1, _2, _3);
		this->getRole().waitResponse(pb::core::OP_ClientLoginResponse, pb::core::OP_AccountLoginRequest, bindCb);
	}

	void LoginTestCase::tearDown()
	{
	}

	std::shared_ptr<TestCase> LoginTestCase::createMethod(MockRole& role, uint32_t type)
	{
		return std::make_shared<LoginTestCase>(role, type);
	}

	uint32_t LoginTestCase::getFactoryType()
	{
		return ETCT_Login;
	}

	void LoginTestCase::handleLogin(MockRole* ptrRole, MessageInfo info, const std::string& msg)
	{
		this->setStatus(ETestCaseStatus::ECS_SUCCESS);
	}
}

