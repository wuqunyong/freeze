#include "logic/test_11/test_module_11.h"




namespace apie {

std::string TestModule11::moduleName()
{
	return "TestModule11";
}

uint32_t TestModule11::modulePrecedence()
{
	return 11;
}

TestModule11::TestModule11(std::string name, module_loader::ModuleLoaderBase* prtLoader)
	: m_name(name),
	m_prtLoader(prtLoader)
{

}

TestModule11::~TestModule11()
{

}

apie::status::Status TestModule11::init()
{
	std::stringstream ss;
	ss << "TestModule11 init";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG(PIE_NOTICE, "ModuleLoad|{}", ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule11::start()
{
	std::stringstream ss;
	ss << "TestModule11 start";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG(PIE_NOTICE, "ModuleLoad|{}", ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule11::ready()
{
	std::stringstream ss;
	ss << "TestModule11 ready";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG(PIE_NOTICE, "ModuleLoad|{}", ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule11::exit()
{
	std::stringstream ss;
	ss << "TestModule11 exit";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG(PIE_NOTICE, "ModuleLoad|{}", ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void TestModule11::setHookReady(hook::HookPoint point)
{
	m_prtLoader->setHookReady(point);
}

}

