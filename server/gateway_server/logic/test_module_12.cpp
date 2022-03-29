#include "logic/test_module_12.h"

#include "../../common/dao/model_user.h"
#include "../../common/dao/model_role_extra.h"
#include "../../common/opcodes.h"



namespace apie {

std::string TestModule12::moduleName()
{
	return "TestModule12";
}

uint32_t TestModule12::modulePrecedence()
{
	return 12;
}

TestModule12::TestModule12(std::string name, module_loader::ModuleLoaderBase* prtLoader)
	: m_name(name),
	m_prtLoader(prtLoader)
{

}

apie::status::Status TestModule12::init()
{
	std::stringstream ss;
	ss << "TestModule12 init";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule12::start()
{
	std::stringstream ss;
	ss << "TestModule12 start";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule12::ready()
{
	std::stringstream ss;
	ss << "TestModule12 ready";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule12::exit()
{
	std::stringstream ss;
	ss << "TestModule12 exit";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void TestModule12::setHookReady(hook::HookPoint point)
{
	m_prtLoader->setHookReady(point);
}

}

