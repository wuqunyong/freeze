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

#include "service_init.h"

#include "boost/pfr.hpp"

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




int main(int argc, char **argv)
{
	some_person val{ "hello", 1809};

	std::cout << boost::pfr::get<0>(val) << " was born in " << boost::pfr::get<1>(val);
	std::cout << boost::pfr::io(val);

	std::vector<uint32_t> offsetV;
	to_vec_impl(offsetV, val, std::make_index_sequence<2>{});

	std::vector<std::set<MysqlField::DB_FIELD_TYPE>> layer;
	to_layer_impl(layer, val, std::make_index_sequence <boost::pfr::tuple_size_v<some_person>>{});

	if (argc != 2)
	{
		PANIC_ABORT("usage: exe <ConfFile>, Expected: %d, got: %d", 2, argc);
	}

	std::string configFile = argv[1];

	apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Init, apie::initHook);
	apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Start, apie::startHook);
	apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Ready, apie::readyHook);
	apie::hook::HookRegistrySingleton::get().registerHook(apie::hook::HookPoint::HP_Exit, apie::exitHook);

	apie::CtxSingleton::get().init(configFile);
	apie::CtxSingleton::get().start();
	apie::CtxSingleton::get().waitForShutdown();

	return 0;
}
