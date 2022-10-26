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

	void loadFromDbLoader(std::shared_ptr<apie::DbLoadComponent> ptrLoader)
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
	}

	void saveToDb()
	{
		std::cout << "ModuleA saveToDb" << std::endl;
	}

private:
	uint64_t m_iId = 0;

	Single_ModelUser_Loader::Type m_data1;
	Single_ModelRoleExtra_Loader::Type m_data2;

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

	void loadFromDbLoader(std::shared_ptr<apie::DbLoadComponent> ptrLoader)
	{
		std::cout << "ModuleB loadFromDbLoad" << std::endl;

		if (ptrLoader->has<Multi_ModelUser_Loader>())
		{
			m_data1 = ptrLoader->get<Multi_ModelUser_Loader>();
		}
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


apie::status::Status TestModule10::ready()
{
	std::stringstream ss;
	ss << "TestModule10 ready";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ModuleLoad", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);

	auto ptrModuleLoader = CreateLoadInstance(123);

	auto& rModuleA = ptrModuleLoader->lookup<TestModuleA>();
	auto sInfo = rModuleA.toString();

	auto& rModuleB = ptrModuleLoader->lookup<TestModuleB>();
	sInfo = rModuleB.toString();
	rModuleB.incrementValue();
	sInfo = rModuleB.toString();

	ptrModuleLoader->saveToDb();

	auto ptrLoad = CreateLoadObj();
	ptrLoad->set<Single_ModelUser_Loader>(1);
	ptrLoad->set<Single_ModelRoleExtra_Loader>(1);
	ptrLoad->set<Multi_ModelUser_Loader>(1).lookup<Multi_ModelUser_Loader>().markFilter({ ModelUser::user_id });

	auto cb = [ptrModuleLoader](apie::status::Status status, std::shared_ptr<apie::DbLoadComponent> loader) {
		if (status.ok())
		{
			ptrModuleLoader->loadFromDbLoader(loader);
		}
	};
	ptrLoad->loadFromDb(server, cb);

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

