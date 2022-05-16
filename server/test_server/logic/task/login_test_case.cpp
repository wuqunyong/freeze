#include "logic/task/login_test_case.h"

#include "../../../common/opcodes.h"

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

		auto bindCb = std::bind(&LoginTestCase::pendingNotify_Cmd_Login_Notice, this, _1, _2, _3);
		m_id = this->getRole().waitResponse(MergeOpcode(::apie::_MSG_TALENT_CMD, pb::talent::E_Talent_Cmd_Login_Notice),
			MergeOpcode(::apie::_MSG_CLIENT_LOGINTOL, 0), bindCb);
	}

	void LoginTestCase::tearDown()
	{
		this->getRole().removePendingNotifyById(m_id);
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

