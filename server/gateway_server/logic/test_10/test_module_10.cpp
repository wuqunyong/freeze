#include "logic/test_10/test_module_10.h"




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
	ASYNC_PIE_LOG(PIE_NOTICE, "ModuleLoad|{}", ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule10::start()
{
	std::stringstream ss;
	ss << "TestModule10 start";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG(PIE_NOTICE, "ModuleLoad|{}", ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}



static auto CreateLoadInstance(uint64_t iId)
{
	static auto tupleType = std::make_tuple(TestModuleA(), TestModuleB());
	auto pInstance = MakeComponentLoader(iId, tupleType);
	return pInstance;
}

//using CreateLoadInstanceCb = std::function<void(apie::status::Status status, decltype(CreateLoadInstance(0)))>;

apie::status::Status TestModule10::ready()
{
	std::stringstream ss;
	ss << "TestModule10 ready";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG(PIE_NOTICE, "ModuleLoad|{}", ss.str().c_str());

	//decltype(CreateLoadInstance(0)) aTest = CreateLoadInstance(123);

	//::rpc_msg::CHANNEL server;
	//server.set_realm(apie::Ctx::getThisChannel().realm());
	//server.set_type(::common::EPT_DB_ROLE_Proxy);
	//server.set_id(1);

	//auto ptrModuleLoader = CreateLoadInstance(123);

	//auto& rModuleA = ptrModuleLoader->lookup<TestModuleA>();
	//auto sInfo = rModuleA.toString();

	//auto& rModuleB = ptrModuleLoader->lookup<TestModuleB>();
	//sInfo = rModuleB.toString();
	//rModuleB.incrementValue();
	//sInfo = rModuleB.toString();

	//ptrModuleLoader->saveToDb();

	//auto ptrLoad = CreateDBLoaderPtr();
	//ptrLoad->set<Single_ModelUser_Loader>(1);
	//ptrLoad->set<Single_ModelRoleExtra_Loader>(1);
	//ptrLoad->set<Multi_ModelUser_Loader>(1).lookup<Multi_ModelUser_Loader>().markFilter({ ModelUser::user_id });

	//auto cb = [ptrModuleLoader](apie::status::Status status, std::shared_ptr<apie::DbLoadComponent> loader) {
	//	if (status.ok())
	//	{
	//		ptrModuleLoader->loadFromDbLoader(loader);
	//	}
	//};
	//ptrLoad->loadFromDb(server, cb);

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestModule10::exit()
{
	std::stringstream ss;
	ss << "TestModule10 exit";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG(PIE_NOTICE, "ModuleLoad|{}", ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void TestModule10::setHookReady(hook::HookPoint point)
{
	m_prtLoader->setHookReady(point);
}

}

