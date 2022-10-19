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

class ModuleLoader
{
public:
	template <typename T>
	using ValueTypeT = typename T::Type;

	template <typename... Arg>
	ModuleLoader(uint64_t iId, Arg&&... a) :
		m_id(iId)
	{  
		AppendAll(std::forward<Arg&&>(a)...);
	}

	template <typename T>
	void Append(T moduleType) 
	{
		m_options.set<T>(m_id);
		m_modules.push_back(typeid(moduleType));
	}

	template <typename T>
	bool has() const 
	{
		return m_options.has<T>();
	}

	template <typename T>
	ValueTypeT<T>& lookup(ValueTypeT<T> value = {})
	{
		if (!has<T>())
		{
			throw std::exception("unregister");
		}

		return m_options.lookup<T>(value);
	}

private:
	ModuleLoader(const ModuleLoader&) = delete;
	ModuleLoader& operator=(const ModuleLoader&) = delete;
	ModuleLoader(ModuleLoader&&) = delete;
	ModuleLoader& operator=(ModuleLoader&&) = delete;


	template <typename H, typename... Tail>
	void AppendAll(H&& head, Tail&&... a) 
	{
		Append(std::forward<H>(head));
		AppendAll(std::forward<Tail>(a)...);
	}

	/// Terminate the recursion.
	void AppendAll() 
	{
	}

	uint64_t m_id = 0;
	std::vector<std::type_index> m_modules;
	apie::common::Options m_options;
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

private:
	uint64_t m_iId = 0;
	uint64_t m_value = 0;
};

struct TestModuleB
{
	using Type = ModuleB;
};

int main(int argc, char **argv)
{

	ModuleLoader moduleLoader(1, TestModuleA(), TestModuleB());

	auto& rModule = moduleLoader.lookup<TestModuleA>();
	auto sInfo = rModule.toString();

	auto& rModuleB = moduleLoader.lookup<TestModuleB>();
	sInfo = rModuleB.toString();
	rModuleB.incrementValue();
	sInfo = rModuleB.toString();



	//uint64_t iId = 123;
	//apie::common::Options m_options;
	//m_options.set<TestModuleA>(iId);
	//auto& rValue = m_options.get<TestModuleA>();

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
