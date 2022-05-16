#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>
#include <memory>
#include <set>

#include "apie.h"

#include "logic/mock_role.h"
#include "logic/test_case.h"


namespace apie {

	class TalentTestCase : public TestCase
	{
	public:
		TalentTestCase(MockRole& role, uint32_t type);

		virtual void setUp();
		virtual void tearDown();

	public:
		void RPC_onOpen(MockRole* ptrRole, MessageInfo info, const std::string& msg);
		void RPC_onChoose(MockRole* ptrRole, MessageInfo info, const std::string& msg);

	public:
		static std::shared_ptr<TestCase> createMethod(MockRole& role, uint32_t type);
		static uint32_t getFactoryType();

	private:
		uint32_t m_id;
	};
}
