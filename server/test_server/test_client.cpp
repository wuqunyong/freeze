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

#include "service_init.h"
#include "logic/test_server.h"

#include "boost/pfr.hpp"

#include "../../pb_msg/business/login_msg.pb.h"


struct some_person {
	std::string name;
	unsigned birth_year;
};


template<typename T, std::size_t... Idx>
void to_vec_impl(std::vector<uint32_t>& vec, T& t, std::index_sequence<Idx...> /*unused*/)
{
	(vec.push_back(reinterpret_cast<char*>(&boost::pfr::get<Idx>(t)) - reinterpret_cast<char*>(&boost::pfr::get<0>(t))), ...);
}


template<typename T, std::size_t... Idx>
void to_layer_impl(std::vector<std::set<MysqlField::DB_FIELD_TYPE>>& vec, T& t, std::index_sequence<Idx...> /*unused*/)
{
	(vec.push_back(get_field_type(boost::pfr::get<Idx>(t))), ...);
}

class ItemsConfigList
{
public:
	uint32_t Id = 0;
	uint32_t Quality = 0;
	uint32_t StackNum = 0;
	uint32_t UseNum = 0;
	uint32_t CoreLv = 0;
	uint32_t Page = 0;
	uint32_t Type = 0;
	uint32_t SubType = 0;
	uint32_t Gift = 0;
	uint32_t Value1 = 0;
	std::string Value2;
	std::optional<std::string> Path;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ItemsConfigList, Id, Quality, StackNum, UseNum, CoreLv, Page, Type, SubType, Gift, Value1, Value2, Path);
};

class ItemsConfig
{
public:
	std::string TableName;
	std::vector<ItemsConfigList> List;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ItemsConfig, TableName, List);
};

class Items
{
public:
	ItemsConfig Config;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Items, Config);


	bool isValid(std::string& errInfo)
	{
		return true;
	}
	static constexpr const char* sName = "items";
};


auto test_json(std::string file_name)
{
	std::string content;
	bool bResult = apie::common::GetContent(file_name, &content);
	if (!bResult)
	{
		return;
	}

	nlohmann::json jsonObj = nlohmann::json::parse(content);
	Items itemsObj = jsonObj.get<Items>();
}

void print_int(std::shared_ptr<apie::service::SyncServiceBase> ptrBase) {
	std::cout << "SyncServiceBase 1" << '\n';
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	std::cout << "SyncServiceBase 2" << '\n';
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));

	auto ptrData = std::make_shared<::login_msg::MSG_REQUEST_ACCOUNT_LOGIN_L>();
	ptrData->set_platform_id("hello world");
	ptrBase->getHandler()(ptrData);
}

int test_func1(int a)
{
	return a;
}

class TestClass1
{
public:
	int test_echo(int a)
	{
		return a;
	}
};

int main(int argc, char **argv)
{
	some_person val{ "hello", 1809};

	int iBodyLen = 8;
	std::string sBody;
	sBody.resize(iBodyLen, '\0');
	strncpy(&sBody[0], "123456789abcdefg", iBodyLen);

	decltype(&test_func1) func_1;
	decltype(test_func1)* func_2 = &test_func1;
	decltype(test_func1)& func_3 = test_func1;

	decltype(&TestClass1::test_echo) mem_1;
	decltype(&TestClass1::test_echo) *mem_2;

	std::cout << typeid(func_1).name() << std::endl;
	std::cout << typeid(func_2).name() << std::endl;
	std::cout << typeid(func_3).name() << std::endl;

	std::cout << typeid(mem_1).name() << std::endl;
	std::cout << typeid(mem_2).name() << std::endl;

	mem_1 = &TestClass1::test_echo;
	TestClass1 tc1;
	(tc1.*mem_1)(100);

	auto ptrFn = &TestClass1::test_echo;
	mem_2 = &ptrFn;
	(tc1.**mem_2)(200);

	std::cout << boost::pfr::get<0>(val) << " was born in " << boost::pfr::get<1>(val);
	std::cout << boost::pfr::io(val);

	test_json("F:/freeze/data/Item.json");

	apie::LCMgrSingleton::get().registerConfig<Items>(Items::sName, "F:/freeze/data/Item.json");
	apie::LCMgrSingleton::get().loadAll();

	auto ptrConfig = apie::LCMgrSingleton::get().getConfig<Items>(Items::sName);

	apie::LCMgrSingleton::get().reloadFile("F:/freeze/data/Item.json");

	std::vector<uint32_t> offsetV;
	to_vec_impl(offsetV, val, std::make_index_sequence<2>{});

	std::vector<std::set<MysqlField::DB_FIELD_TYPE>> layer;
	to_layer_impl(layer, val, std::make_index_sequence <boost::pfr::tuple_size_v<some_person>>{});

	if (argc != 2)
	{
		PANIC_ABORT("usage: exe <ConfFile>, Expected: %d, got: %d", 2, argc);
	}

	std::string configFile = argv[1];

	apie::module_loader::ModuleLoaderMgrSingleton::get().registerModule<apie::TestServerMgr>();

	apie::hook::APieModuleObj(apie::APieModuleObj);

	apie::CtxSingleton::get().init(configFile);
	apie::CtxSingleton::get().start();
	apie::CtxSingleton::get().waitForShutdown();

	return 0;
}
