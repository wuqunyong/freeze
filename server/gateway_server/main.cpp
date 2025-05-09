#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "logic/init_service/gateway_mgr.h"
#include "logic/test_10/test_module_10.h"
#include "logic/test_11/test_module_11.h"
#include "logic/test_12/test_module_12.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		PANIC_ABORT("usage: exe <ConfFile>, Expected: {}, got: {}", 2, argc);
	}

	std::string configFile = argv[1];

	RegisterModule<apie::GatewayMgr>();
	RegisterModule<apie::TestModule10>();
	RegisterModule<apie::TestModule11>();
	RegisterModule<apie::TestModule12>();

	apie::CtxSingleton::get().init(configFile);
	apie::CtxSingleton::get().start();
	apie::CtxSingleton::get().waitForShutdown();

    return 0;
}
