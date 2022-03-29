#include "logic/test_module_10.h"

#include "../../common/dao/model_user.h"
#include "../../common/dao/model_role_extra.h"
#include "../../common/opcodes.h"



namespace apie {

std::string TestModule10::moduleName()
{
	return "TestModule10";
}

uint32_t TestModule10::modulePrecedence()
{
	return 10;
}

TestModule10::TestModule10(std::string name, module_loader::ModuleLoaderBase* prtLoader)
	: m_name(name),
	m_prtLoader(prtLoader)
{

}

apie::status::Status TestModule10::init()
{
	std::stringstream ss;
	ss << "TestModule10 init";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule10::start()
{
	std::stringstream ss;
	ss << "TestModule10 start";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule10::ready()
{
	std::stringstream ss;
	ss << "TestModule10 ready";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule10::exit()
{
	std::stringstream ss;
	ss << "TestModule10 exit";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void TestModule10::setHookReady(hook::HookPoint point)
{
	m_prtLoader->setHookReady(point);
}

}

