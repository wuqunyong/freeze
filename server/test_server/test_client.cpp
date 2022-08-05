#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <type_traits>
#include <variant>
#include <utility>
#include <nlohmann/json.hpp>
#include <thread>
#include <optional>
#include <typeinfo>
#include <typeindex>

#include "apie.h"

#include "logic/test_server.h"
#include "component.h"

#include "../common/dao/model_user.h"
#include "../common/dao/model_account.h"


struct StringOption {
	using Type = std::string;
};


template <typename T>
struct SingleRow {
	using TableType = T;

	SingleRow(uint32_t id = 0) :
		m_tableType(id)
	{

	}

	void loadFromDb() 
	{
		::rpc_msg::CHANNEL server;
		server.set_realm(apie::Ctx::getThisChannel().realm());
		server.set_type(::common::EPT_DB_ACCOUNT_Proxy);
		server.set_id(1);


		auto cb = [](apie::status::Status status, TableType account, uint32_t iRows) {
			if (!status.ok())
			{
				return;
			}
		};
		apie::LoadFromDb<TableType>(server, m_tableType, cb);
	}

	TableType m_tableType;
	std::optional<TableType> m_data;

	inline static std::string m_tableName = TableType::getFactoryName();
};

template <typename T>
struct MultiRow {
	using TableType = T;

	MultiRow(uint32_t id = 0) :
		m_tableType(id)
	{

	}

	void loadFromDb()
	{
		::rpc_msg::CHANNEL server;
		server.set_realm(apie::Ctx::getThisChannel().realm());
		server.set_type(::common::EPT_DB_ACCOUNT_Proxy);
		server.set_id(1);


		auto cb = [](apie::status::Status status, TableType account, uint32_t iRows) {
			if (!status.ok())
			{
				return;
			}
		};
		apie::LoadFromDb<TableType>(server, m_tableType, cb);
	}

	TableType m_tableType;
	std::vector<TableType> m_data;

	inline static std::string m_tableName = TableType::getFactoryName();
};


struct DBUserComponet
{
	using Type = SingleRow<apie::ModelUser>;
};

struct DBAccountComponet
{
	using Type = SingleRow<apie::ModelAccount>;
};

struct DBAccountMultiComponet
{
	using Type = MultiRow<apie::ModelAccount>;
};

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		PANIC_ABORT("usage: exe <ConfFile>, Expected: %d, got: %d", 2, argc);
	}


	bool bResult = false;

	apie::Component component;

	component.set<DBUserComponet>(SingleRow<apie::ModelUser>(1));
	bResult = component.has<DBUserComponet>();
	component.lookup<DBUserComponet>().loadFromDb();

	component.set<DBAccountComponet>(SingleRow<apie::ModelAccount>(1));
	bResult = component.has<DBAccountComponet>();
	component.lookup<DBAccountComponet>().loadFromDb();

	component.set<DBAccountMultiComponet>(MultiRow<apie::ModelAccount>(1));
	bResult = component.has<DBAccountMultiComponet>();
	component.lookup<DBAccountMultiComponet>().loadFromDb();


	APieRegisterModule<apie::TestServerMgr>();

	std::string configFile = argv[1];
	apie::CtxSingleton::get().init(configFile);
	apie::CtxSingleton::get().start();
	apie::CtxSingleton::get().waitForShutdown();

	return 0;
}
