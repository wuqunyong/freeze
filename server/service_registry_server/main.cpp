#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "service_init.h"
#include "logic/service_registry.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		PANIC_ABORT("usage: exe <ConfFile>, Expected: %d, got: %d", 2, argc);
	}

	std::string configFile = argv[1];

	apie::module_loader::ModuleLoaderMgrSingleton::get().registerModule<apie::ServiceRegistry>();

	apie::hook::APieModuleObj(apie::APieModuleObj);

	apie::CtxSingleton::get().init(configFile);
	apie::CtxSingleton::get().start();
	apie::CtxSingleton::get().waitForShutdown();

    return 0;
}
