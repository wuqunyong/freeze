#include "logic/task/logout_test_case.h"


namespace apie {
	using namespace std::placeholders;

	LogoutTestCase::LogoutTestCase(MockRole& role, uint32_t type) :
		TestCase(role, type)
	{

	}

	void LogoutTestCase::setUp()
	{
		::pubsub::TEST_CMD newMsg;
		newMsg.set_module_name("logout");
		newMsg.set_cmd("logout");
		this->getRole().pushMsg(newMsg);
	}

	void LogoutTestCase::tearDown()
	{
	}

	void LogoutTestCase::run()
	{
		TestCase::run();
		this->setStatus(ETestCaseStatus::ECS_SUCCESS);
	}

	std::shared_ptr<TestCase> LogoutTestCase::createMethod(MockRole& role, uint32_t type)
	{
		return std::make_shared<LogoutTestCase>(role, type);
	}

	uint32_t LogoutTestCase::getFactoryType()
	{
		return ETCT_Logout;
	}

}

