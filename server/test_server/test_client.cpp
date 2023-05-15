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


int main(int argc, char **argv)
{
	if (argc != 2)
	{
		PANIC_ABORT("usage: exe <ConfFile>, Expected: %d, got: %d", 2, argc);
	}

	std::string configFile = argv[1];
	apie::CtxSingleton::get().init(configFile);
	apie::CtxSingleton::get().start();
	apie::CtxSingleton::get().waitForShutdown();

	return 0;
}
