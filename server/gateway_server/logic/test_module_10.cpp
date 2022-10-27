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

		if (ptrLoader->has<Single_ModelAccount_Loader>())
		{
			m_data3 = ptrLoader->get<Single_ModelAccount_Loader>();
		}
	}

	void loadFromDbDone()
	{
		std::cout << "ModuleA loadFromDbDone" << std::endl;
	}

	void saveToDb()
	{
		::rpc_msg::CHANNEL server;
		server.set_realm(apie::Ctx::getThisChannel().realm());
		server.set_type(::common::EPT_DB_ACCOUNT_Proxy);
		server.set_id(1);

		if (!m_data3.m_data.has_value())
		{
			m_data3.m_data = Single_ModelAccount_Loader::Type::TableType(m_iId);

			auto cb = [](status::Status status, bool result, uint64_t affectedRows, uint64_t insertId) mutable {
				if (!status.ok())
				{
					return;
				}
			};
			InsertToDb<Single_ModelAccount_Loader::Type::TableType>(server, m_data3.m_data.value(), cb);
		}
		else
		{
			m_data3.m_data.value().fields.modified_time++;
			m_data3.m_data.value().markDirty({ ModelAccount::modified_time });
			UpdateToDb<Single_ModelAccount_Loader::Type::TableType>(server, m_data3.m_data.value(), nullptr);

			//auto cb = [](status::Status status, bool result, uint64_t affectedRows) {
			//	if (!status.ok())
			//	{
			//		return;
			//	}
			//};
			//UpdateToDb<Single_ModelAccount_Loader::Type::TableType>(server, m_data3.m_data.value(), cb);
		}
		

		std::cout << "ModuleA saveToDb" << std::endl;
	}

private:
	uint64_t m_iId = 0;
	

	Single_ModelUser_Loader::Type m_data1;
	Single_ModelRoleExtra_Loader::Type m_data2;

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
	}

	void loadFromDbDone()
	{
		std::cout << "ModuleB loadFromDbDone" << std::endl;
	}

	void saveToDb()
	{
		std::cout << "ModuleB saveToDb" << std::endl;
	}

private:
	uint64_t m_iId = 0;
	uint64_t m_value = 0;

	Multi_ModelUser_Loader::Type m_data1;
};

struct TestModuleB
{
	using Type = ModuleB;
};


static auto CreateLoadInstance(uint64_t iId)
{
	static auto tupleType = std::make_tuple(TestModuleA(), TestModuleB());
	auto pInstance = CreateModuleLoaderInstance(iId, tupleType, std::make_index_sequence<std::tuple_size<decltype(tupleType)>::value>{});
	return pInstance;
}

auto CreateUserObj(uint64_t iRoleId, std::function<void(apie::status::Status status, decltype(CreateLoadInstance(0)))> doneCb)
{
	auto ptrModuleLoader = CreateLoadInstance(iRoleId);

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto ptrLoad = CreateDBLoaderPtr();
	ptrLoad->set<Single_ModelUser_Loader>(iRoleId);
	ptrLoad->set<Single_ModelRoleExtra_Loader>(iRoleId);
	ptrLoad->set<Multi_ModelUser_Loader>(iRoleId).lookup<Multi_ModelUser_Loader>().markFilter({ ModelUser::user_id });

	auto cb = [ptrModuleLoader, doneCb, server, iRoleId](apie::status::Status status, std::shared_ptr<apie::DbLoadComponent> loader) {
		if (!status.ok())
		{
			doneCb(status, ptrModuleLoader);
			return;
		}

		ptrModuleLoader->loadFromDbLoader(server, loader);

		::rpc_msg::CHANNEL accountServer;
		accountServer.set_realm(apie::Ctx::getThisChannel().realm());
		accountServer.set_type(::common::EPT_DB_ACCOUNT_Proxy);
		accountServer.set_id(1);

		loader->clear();
		loader->set<Single_ModelAccount_Loader>(iRoleId);

		auto wrapperFunc = [loader, ptrModuleLoader, doneCb, accountServer]() mutable {
			auto funObj = [ptrModuleLoader, doneCb, accountServer](apie::status::Status status, std::shared_ptr<apie::DbLoadComponent> ptrLoader) mutable {
				if (!status.ok())
				{
					doneCb(status, ptrModuleLoader);
					return;
				}

				ptrModuleLoader->loadFromDbLoader(accountServer, ptrLoader);
				ptrModuleLoader->loadFromDbDone();
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

			ptrModule->saveToDb();
		}
	};
	CreateUserObj(30003, doneCb);

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

