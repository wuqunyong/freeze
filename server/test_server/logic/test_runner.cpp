#include "logic/test_runner.h"

#include "../module_handler/test_server_module.h"

namespace apie {
	TestRunner::TestRunner()
	{
		auto runCb = [this]() {
				this->run();

				if (this->m_done)
				{
					this->m_runTimer->disableTimer();
				}
				else
				{
					this->m_runTimer->enableTimer(std::chrono::milliseconds(10));
				}
		};
		this->m_runTimer = apie::CtxSingleton::get().getLogicThread()->dispatcher().createTimer(runCb);
	}

	void TestRunner::init()
	{
		if (m_init)
		{
			return ;
		}

		this->m_startTime = Ctx::getCurMilliseconds();
		this->m_runTimer->enableTimer(std::chrono::milliseconds(10));

		m_init = true;
		bool bResult = loadConfig();

		m_enable = bResult;
		if (m_enable)
		{
			m_activePos = m_start;
		}

		TestServerModule::registerTestCase();
	}

	void TestRunner::run()
	{
		if (!m_enable)
		{
			return;
		}

		if (m_done)
		{
			return;
		}

		auto iCurMs = Ctx::getCurMilliseconds();
		if (m_nextRampUpTimes == 0)
		{
			m_nextRampUpTimes = iCurMs;
		}

		do 
		{
			if (iCurMs < m_nextRampUpTimes)
			{
				break;
			}

			m_nextRampUpTimes = iCurMs + m_rampUpIntervalMs;

			uint32_t iCount = 0;
			while (m_activePos < m_stop)
			{
				iCount++;

				auto findIte = m_activeSuite.find(m_activePos);
				if (findIte == m_activeSuite.end())
				{
					auto prtSuite = TestSuite::makeTestSuite(m_activePos);
					if (prtSuite != nullptr)
					{
						m_activeSuite[m_activePos] = prtSuite;
						prtSuite->setUp();

						std::stringstream ss;
						ss << "roleId:" << m_activePos << "|suite start ";
						ASYNC_PIE_LOG(PIE_DEBUG, "makeTestSuite:{}", ss.str());
						std::cout << ss.str() << std::endl;
					}
					else
					{
						std::stringstream ss;
						ss << "roleId:" << m_activePos << "|makeTestSuite error ";
						ASYNC_PIE_LOG(PIE_DEBUG, "makeTestSuite:{}", ss.str());
						std::cout << ss.str() << std::endl;
					}
				}

				m_activePos++;
				if (iCount >= m_rampUpNums)
				{
					break;
				}
			}
		} while (false);

		std::set<uint64_t> doneList;

		for (auto& elems : m_activeSuite)
		{
			elems.second->run();

			if (elems.second->isComplete())
			{
				doneList.insert(elems.first);
			}
		}

		for (auto& elems : doneList)
		{
			auto findIte = m_activeSuite.find(elems);
			if (findIte != m_activeSuite.end())
			{
				findIte->second->tearDown();

				std::stringstream ss;
				ss << "roleId:" << elems << "|suite done ";
				ASYNC_PIE_LOG(PIE_DEBUG, "makeTestSuite:{}", ss.str());

				std::cout << ss.str() << std::endl;

				m_doneSuite[elems] = findIte->second;
				m_activeSuite.erase(findIte);
			}
		}

		auto iTotalCount = m_stop - m_start ;
		if (m_activeSuite.empty()
			&& m_doneSuite.size() >= iTotalCount)
		{
			m_done = true;

			auto iEndTime = Ctx::getCurMilliseconds();
			auto iDelta = iEndTime - this->m_startTime;

			//Êä³öREPORT
			std::stringstream ss;

			Json::Value root;
			root["startTime"] = this->m_startTime;
			root["endTime"] = iEndTime;
			root["durationTime"] = iDelta;
			root["roleStart"] = m_start;
			root["roleEnd"] = m_stop;
			root["rampUpIntervalMs"] = m_rampUpIntervalMs;
			root["rampUpNums"] = m_rampUpNums;

			Json::Value role;
			for (auto& elems : m_doneSuite)
			{
				uint64_t iRoleId = elems.second->getRoleId();
				std::string sRoleId = std::to_string(iRoleId);

				role[sRoleId] = elems.second->getReport();
			}
			root["role"] = role;

			ss << root << std::endl;

			ASYNC_PIE_LOG(PIE_DEBUG, "TestRunner:{}:{}", "TestRunner Result", ss.str().c_str());

			std::cout << "TestRunner Result:" << ss.str() << std::endl;

			Json::StreamWriterBuilder builder;
			const std::string json_file = Json::writeString(builder, root);


			ss.str("");
			ss << "report/report_" << m_startTime;
			std::string fileName = ss.str();

			//ASYNC_PIE_LOG_CUSTOM(fileName.c_str(), PIE_CYCLE_DAY, PIE_NOTICE, "%s", json_file.c_str());


			for (auto& elems : m_doneSuite)
			{
				elems.second->clear();
			}

		}
	}

	bool TestRunner::loadConfig()
	{
		bool enable = apie::CtxSingleton::get().getConfigs()->auto_test.enable;
		if (enable)
		{
			m_start = apie::CtxSingleton::get().getConfigs()->auto_test.start;
			m_stop = apie::CtxSingleton::get().getConfigs()->auto_test.stop;
			m_rampUpIntervalMs = apie::CtxSingleton::get().getConfigs()->auto_test.ramp_up_interval;
			m_rampUpNums = apie::CtxSingleton::get().getConfigs()->auto_test.ramp_up_nums;

			return true;
		}
		else
		{
			return false;
		}
	}

}

