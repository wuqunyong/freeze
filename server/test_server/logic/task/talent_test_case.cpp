#include "logic/task/talent_test_case.h"

#include "../../../common/opcodes.h"

namespace apie {
	using namespace std::placeholders;

	TalentTestCase::TalentTestCase(MockRole& role, uint32_t type) :
		TestCase(role, type)
	{

	}

	void TalentTestCase::setUp()
	{
		::pb::talent::Talent_Open_Req request;
		auto iRequestId = MergeOpcode(::apie::_MSG_TALENT_CMD, ::pb::talent::E_Talent_Cmd_Open_Req);
		uint64_t iRpcId = getRole().sendMsg(iRequestId, request);

		auto bindCb = std::bind(&TalentTestCase::RPC_onOpen, this, _1, _2, _3);
		this->getRole().waitRPC(iRpcId, iRequestId, bindCb);
	}

	void TalentTestCase::tearDown()
	{

	}

	std::shared_ptr<TestCase> TalentTestCase::createMethod(MockRole& role, uint32_t type)
	{
		return std::make_shared<TalentTestCase>(role, type);
	}

	uint32_t TalentTestCase::getFactoryType()
	{
		return ETCT_Talent;
	}

	void TalentTestCase::RPC_onOpen(MockRole* ptrRole, MessageInfo info, const std::string& msg)
	{
		pb::talent::Talent_Open_Response response;
		bool bResult = response.ParseFromString(msg);
		if (!bResult)
		{
			this->setStatus(ETestCaseStatus::ECS_FAILURE);
			return;
		}

		switch (response.error_code())
		{
		case 0:
		{
			uint32_t iCount = 0;

			::pb::talent::Talent_Choose_Req request;
			for (const auto& elems : response.data())
			{
				uint32_t id = elems;
				request.add_id(id);

				m_id = id / 10;
				
				iCount++;
				if (iCount >= response.reserve_num())
				{
					break;
				}
			}

			auto iRequestId = MergeOpcode(::apie::_MSG_TALENT_CMD, ::pb::talent::E_Talent_Cmd_Choose_Req);
			uint64_t iRpcId = getRole().sendMsg(iRequestId, request);

			auto bindCb = std::bind(&TalentTestCase::RPC_onChoose, this, _1, _2, _3);
			this->getRole().waitRPC(iRpcId, iRequestId, bindCb);
			break;
		}
		default:
		{
			this->setStatus(ETestCaseStatus::ECS_FAILURE);
			break;
		}
		}
	}

	void TalentTestCase::RPC_onChoose(MockRole* ptrRole, MessageInfo info, const std::string& msg)
	{
		pb::talent::Talent_Choose_Response response;
		bool bResult = response.ParseFromString(msg);
		if (!bResult)
		{
			this->setStatus(ETestCaseStatus::ECS_FAILURE);
			return;
		}

		if (response.error_code() != 0)
		{
			this->setStatus(ETestCaseStatus::ECS_FAILURE);
			return;
		}

		::pb::talent::Talent_Activate_Req request;
		request.set_id(m_id);

		auto iRequestId = MergeOpcode(::apie::_MSG_TALENT_CMD, ::pb::talent::E_Talent_Cmd_Activate_Req);
		uint64_t iRpcId = getRole().sendMsg(iRequestId, request);

		auto bindCb = std::bind(&TalentTestCase::RPC_onActivate, this, _1, _2, _3);
		this->getRole().waitRPC(iRpcId, iRequestId, bindCb);
	}

	void TalentTestCase::RPC_onActivate(MockRole* ptrRole, MessageInfo info, const std::string& msg)
	{
		pb::talent::Talent_Activate_Response response;
		bool bResult = response.ParseFromString(msg);
		if (!bResult)
		{
			this->setStatus(ETestCaseStatus::ECS_FAILURE);
			return;
		}

		if (response.error_code() != 0)
		{
			this->setStatus(ETestCaseStatus::ECS_FAILURE);
			return;
		}

		this->setStatus(ETestCaseStatus::ECS_SUCCESS);
	}

}


