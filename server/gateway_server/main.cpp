#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "service_init.h"

#include "logic/gateway_mgr.h"
#include "logic/test_module_10.h"
#include "logic/test_module_11.h"
#include "logic/test_module_12.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		PANIC_ABORT("usage: exe <ConfFile>, Expected: %d, got: %d", 2, argc);
	}

	std::string configFile = argv[1];

	apie::module_loader::ModuleLoaderMgrSingleton::get().registerModule<apie::GatewayMgr>();
	apie::module_loader::ModuleLoaderMgrSingleton::get().registerModule<apie::TestModule10>();
	apie::module_loader::ModuleLoaderMgrSingleton::get().registerModule<apie::TestModule11>();
	apie::module_loader::ModuleLoaderMgrSingleton::get().registerModule<apie::TestModule12>();

	apie::hook::APieModuleObj(apie::APieModuleObj);

	apie::CtxSingleton::get().init(configFile);
	apie::CtxSingleton::get().start();
	apie::CtxSingleton::get().waitForShutdown();

    return 0;
}
