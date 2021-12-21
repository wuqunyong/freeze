#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "service_init.h"


int main(int argc, char **argv)
{
	if (argc != 2)
	{
		PANIC_ABORT("usage: exe <ConfFile>, Expected: %d, got: %d", 2, argc);
	}

	std::string configFile = argv[1];

	apie::hook::APieModuleInit(apie::initHook);
	apie::hook::APieModuleStart(apie::startHook);
	apie::hook::APieModuleReady(apie::readyHook);
	apie::hook::APieModuleExit(apie::exitHook);

	apie::CtxSingleton::get().init(configFile);
	apie::CtxSingleton::get().start();
	apie::CtxSingleton::get().waitForShutdown();

    return 0;
}
