#include "logic/test_10/test_module_10.h"

#include "../../../common/opcodes.h"



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


class ModuleA
{
public:
	ModuleA(uint64_t iId = 0) :
		m_iId(iId)
	{

	}

	std::string toString()
	{
		return "ModuleA";
	}

	void loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader)
	{
		std::cout << "ModuleA loadFromDbLoad" << std::endl;

		if (ptrLoader->has<Single_ModelUser_Loader>())
		{
			m_data1 = ptrLoader->get<Single_ModelUser_Loader>();
		}

		if (ptrLoader->has<Single_ModelRoleExtra_Loader>())
		{
			m_data2 = ptrLoader->get<Single_ModelRoleExtra_Loader>();
		}

		if (ptrLoader->has<Single_ModelVarchars1_Loader>())
		{
			m_data10 = ptrLoader->get<Single_ModelVarchars1_Loader>();
		}

		if (ptrLoader->has<Single_ModelAccount_Loader>())
		{
			m_data3 = ptrLoader->get<Single_ModelAccount_Loader>();
		}
	}

	void loadFromDbDone()
	{
		if (!m_data3.m_optData.has_value())
		{
			m_data3.m_optData = Single_ModelAccount_Loader::Type::TableType(m_iId);
			InsertToDb<Single_ModelAccount_Loader::Type::TableType>(m_data3.m_server, m_data3.m_optData.value(), nullptr);
		}

		std::cout << "ModuleA loadFromDbDone" << std::endl;
	}

	void saveToDb()
	{
		if (m_data3.m_optData.has_value())
		{
			m_data3.m_optData.value().set_modified_time(time(nullptr));
			UpdateToDb<Single_ModelAccount_Loader::Type::TableType>(m_data3.m_server, m_data3.m_optData.value(), nullptr);
		}

		std::cout << "ModuleA saveToDb" << std::endl;
	}

private:
	uint64_t m_iId = 0;
	

	Single_ModelUser_Loader::Type m_data1;
	Single_ModelRoleExtra_Loader::Type m_data2;
	Single_ModelVarchars1_Loader::Type m_data10;

	Single_ModelAccount_Loader::Type m_data3;

};

struct TestModuleA
{
	using Type = ModuleA;
};


class ModuleB
{
public:
	ModuleB(uint64_t iId = 0) :
		m_iId(iId)
	{

	}

	std::string toString()
	{
		std::stringstream ss;
		ss << "ModuleB" << ":" << m_iId << ":" << m_value;
		return ss.str();
	}

	void incrementValue()
	{
		m_value++;
	}

	void loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader)
	{
		std::cout << "ModuleB loadFromDbLoad" << std::endl;

		if (ptrLoader->has<Multi_ModelUser_Loader>())
		{
			m_data1 = ptrLoader->get<Multi_ModelUser_Loader>();
		}

		if (ptrLoader->has<All_ModelAccountName_Loader>())
		{
			m_data2 = ptrLoader->get<All_ModelAccountName_Loader>();
		}
	}

	void loadFromDbDone()
	{
		std::cout << "ModuleB loadFromDbDone" << std::endl;
	}

	void saveToDb()
	{
		std::cout << "ModuleB saveToDb" << std::endl;

		for (auto& elems : m_data2.m_vecData)
		{
			auto name = elems.get_name() + "_test";
			elems.set_name(name);
			UpdateToDb(m_data2.m_server, elems, nullptr);
		}
	}

private:
	uint64_t m_iId = 0;
	uint64_t m_value = 0;
	      
	Multi_ModelUser_Loader::Type m_data1;
	All_ModelAccountName_Loader::Type m_data2;
};

struct TestModuleB
{
	using Type = ModuleB;
};


static auto CreateLoadInstance(uint64_t iId)
{
	static auto tupleType = std::make_tuple(TestModuleA(), TestModuleB());
	auto pInstance = MakeCommonModuleLoader(iId, tupleType);
	return pInstance;
}

using CreateLoadInstanceCb = std::function<void(apie::status::Status status, decltype(CreateLoadInstance(0)))>;

void CreateUserObj(uint64_t iRoleId, CreateLoadInstanceCb doneCb)
{
	auto ptrModuleLoader = CreateLoadInstance(iRoleId);

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto ptrLoad = CreateDBLoaderPtr();
	ptrLoad->set<Single_ModelUser_Loader>(iRoleId);
	ptrLoad->set<Single_ModelRoleExtra_Loader>(iRoleId);
	ptrLoad->set<Single_ModelVarchars1_Loader>({ iRoleId,1 });
	ptrLoad->set<Multi_ModelUser_Loader>(iRoleId).lookup<Multi_ModelUser_Loader>().markFilter({ apie::dbt_role::role_base_AutoGen::user_id });

	auto cb = [ptrModuleLoader, doneCb, server, iRoleId](apie::status::Status status, std::shared_ptr<apie::DbLoadComponent> loader) {
		if (!status.ok())
		{
			doneCb(status, ptrModuleLoader);
			return;
		}

		ptrModuleLoader->Meta_loadFromDbLoader(server, loader);

		::rpc_msg::CHANNEL accountServer;
		accountServer.set_realm(apie::Ctx::getThisChannel().realm());
		accountServer.set_type(::common::EPT_DB_ACCOUNT_Proxy);
		accountServer.set_id(1);

		loader->clear();
		loader->set<Single_ModelAccount_Loader>(iRoleId);
		loader->set<All_ModelAccountName_Loader>(iRoleId);

		auto wrapperFunc = [loader, ptrModuleLoader, doneCb, accountServer]() mutable {
			auto funObj = [ptrModuleLoader, doneCb, accountServer](apie::status::Status status, std::shared_ptr<apie::DbLoadComponent> ptrLoader) mutable {
				if (!status.ok())
				{
					doneCb(status, ptrModuleLoader);
					return;
				}

				ptrModuleLoader->Meta_loadFromDbLoader(accountServer, ptrLoader);
				ptrModuleLoader->Meta_loadFromDbDone();
				doneCb(status, ptrModuleLoader);
			};
			loader->loadFromDb(accountServer, funObj);
		};
		apie::CtxSingleton::get().getLogicThread()->dispatcher().post(wrapperFunc);
	};
	ptrLoad->loadFromDb(server, cb);
}

apie::status::Status TestModule10::ready()
{
	std::stringstream ss;
	ss << "TestModule10 ready";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	//decltype(CreateLoadInstance(0)) aTest = CreateLoadInstance(123);

	auto doneCb = [](apie::status::Status status, auto ptrModule) {
		if (status.ok())
		{
			auto& rModuleA = ptrModule->lookup<TestModuleA>();
			auto sInfo = rModuleA.toString();

			auto& rModuleB = ptrModule->lookup<TestModuleB>();
			sInfo = rModuleB.toString();
			rModuleB.incrementValue();
			sInfo = rModuleB.toString();

			ptrModule->Meta_saveToDb();
		}
	};
	CreateUserObj(30005, doneCb);

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
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void TestModule10::setHookReady(hook::HookPoint point)
{
	m_prtLoader->setHookReady(point);
}

}

