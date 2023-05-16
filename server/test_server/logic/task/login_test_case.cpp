#include "logic/task/login_test_case.h"


namespace apie {
	using namespace std::placeholders;

	LoginTestCase::LoginTestCase(MockRole& role, uint32_t type) :
		TestCase(role, type)
	{

	}

	void LoginTestCase::setUp()
	{
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

	void LoginTestCase::pendingNotify_Cmd_Login_Notice(MockRole* ptrRole, MessageInfo info, const std::string& msg)
	{
		this->setStatus(ETestCaseStatus::ECS_SUCCESS);
	}
}

