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
#include <memory>

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

class TestB;

class TestA
{
public:
	TestA(int a) :
		a_(a)
	{

	}

	~TestA()
	{

	}

	void SetPtr(std::shared_ptr<TestB> ptr)
	{
		ptr_ = ptr;
	}

	std::shared_ptr<TestB> ptr_;
	int a_;
};


class TestB
{
public:
	TestB(int b, std::shared_ptr<TestA> ptr) :
		b_(b),
		ptr_(ptr)
	{

	}

	~TestB()
	{

	}

	std::shared_ptr<TestA> ptr_;
	int b_;
};


class TestC
{
public:
	static std::shared_ptr<TestC> Create(int a)
	{
		return std::shared_ptr<TestC>(new TestC(a));
	}

private: 
	TestC(int a) :
		a_(a)
	{

	}

	int a_;
};

class RowSet {
public:
	/// Create an empty set.
	RowSet() = default;

	RowSet(RowSet&&) = default;
	RowSet& operator=(RowSet&&) = default;
	RowSet(RowSet const&) = default;
	RowSet& operator=(RowSet const&) = default;

	template <typename... Arg>
	RowSet(Arg&&... a) 
	{  // NOLINT(google-explicit-constructor)
		std::tuple<Arg...> m_types = { std::forward<Arg&&>(a)... };


		//uint32_t iSize = std::tuple_size<decltype(m_types)>::value;
		//for (uint32_t i = 0; i < iSize; i++)
		//{
		//	typename std::remove_reference<decltype(std::get<i>(m_types))>::type va;
		//	std::type_index m_typeIndex = typeid(m_types);
		//	m_typeVec.push_back(m_typeIndex);
		//}



		std::type_index m_typeIndex = typeid(m_types);
		auto m_typeName = typeid(m_types).name();

		AppendAll(std::forward<Arg&&>(a)...);
	}


	/**
	 * Add @p row_key to the set, minimize copies when possible.
	 */
	template <typename T>
	void Append(T&& row_key) {
		std::type_index m_typeIndex = typeid(row_key);
		std::string m_type = typeid(row_key).name();

		m_typeVec.push_back(m_typeIndex);
		m_typeName[m_typeIndex] = m_type;
	}

	std::string ToString()
	{
		return "";
	}

private:
	/// Append the arguments to the rowset.
	template <typename H, typename... Tail>
	void AppendAll(H&& head, Tail&&... a) {
		// We cannot use the initializer list expression here because the types
		// may be all different.
		Append(std::forward<H>(head));
		AppendAll(std::forward<Tail>(a)...);
	}

	/// Terminate the recursion.
	void AppendAll() {}

	std::vector<std::type_index> m_typeVec;
	std::unordered_map<std::type_index, std::string> m_typeName;

	//std::type_index m_typeIndex;
	//std::string m_typeName;
};

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

	void saveToDb()
	{
		std::cout << "ModuleA saveToDb" << std::endl;
	}

private:
	uint64_t m_iId = 0;
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

	void saveToDb()
	{
		std::cout << "ModuleB saveToDb" << std::endl;
	}

private:
	uint64_t m_iId = 0;
	uint64_t m_value = 0;
};

struct TestModuleB
{
	using Type = ModuleB;
};


static auto CreateLoadInstance(uint64_t iId)
{
	static auto tupleType = std::make_tuple(TestModuleA(), TestModuleB());
	auto pInstance = apie::CreateCommonModuleLoaderPtr(iId, tupleType, std::make_index_sequence<std::tuple_size<decltype(tupleType)>::value>{});
	return pInstance;
}


CoTaskVoid TestCoRPC()
{
	int i = 0;
	i++;

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ACCOUNT_Proxy);
	server.set_id(1);

	::mysql_proxy_msg::MysqlDescribeRequest args;
	auto ptrAdd = args.add_names();
	*ptrAdd = "test";

	//auto ptrAwait = std::make_shared<apie::co_traits::CoAwaitable<::mysql_proxy_msg::MysqlDescribeRequest, ::mysql_proxy_msg::MysqlDescribeResponse>>(server, rpc_msg::RPC_MysqlDescTable, args);
	//auto response = co_await *ptrAwait;
	
	auto ptrAwait = MakeCoAwaitable<::mysql_proxy_msg::MysqlDescribeRequest, ::mysql_proxy_msg::MysqlDescribeResponse>(server, rpc_msg::RPC_MysqlDescTable, args);
	auto response = co_await *ptrAwait;
	if (!response.ok())
	{
		co_return;
	}

	auto valueObj = response.value();

	i++;

	co_return;
}

int main(int argc, char **argv)
{
	//auto ptrModuleLoader = ModuleLoader::CreateInstance(123);

	TestCoRPC();

	auto ptrModuleLoader = CreateLoadInstance(123);

	auto& rModuleA = ptrModuleLoader->lookup<TestModuleA>();
	auto sInfo = rModuleA.toString();

	auto& rModuleB = ptrModuleLoader->lookup<TestModuleB>();
	sInfo = rModuleB.toString();
	rModuleB.incrementValue();
	sInfo = rModuleB.toString();

	ptrModuleLoader->saveToDb();

	uint64_t iId = 123;
	apie::common::Options m_options;
	m_options.set<TestModuleA>(iId);
	auto& rValue = m_options.get<TestModuleA>();

	if (argc != 2)
	{
		PANIC_ABORT("usage: exe <ConfFile>, Expected: %d, got: %d", 2, argc);
	}

	auto ptrC = TestC::Create(100);
	//auto ptrCC = new TestC(200);


	{
		auto ptrA = std::make_shared<TestA>(100);
		auto ptrB = std::make_shared<TestB>(200, ptrA);
		//ptrA->SetPtr(ptrB);

		ptrA = nullptr;
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
