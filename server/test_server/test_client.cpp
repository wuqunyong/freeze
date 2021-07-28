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

#include "service_init.h"


template <size_t I = 0, typename... Ts>
constexpr void printTuple(std::tuple<Ts...> tup, std::map<uint32_t, uint32_t> index)
{
	// If we have iterated through all elements
	if
		constexpr (I == sizeof...(Ts))
	{
		// Last case, if nothing is left to
		// iterate, then exit the function
		return;
	}
	else {
		// Print the tuple and go to next element
		if (index[I] > 0)
		{
			std::cout << "index:" << I  << "|" << std::get<I>(tup) << " ";
		}

		// Going for next element.
		printTuple<I + 1>(tup, index);
	}
}

int main(int argc, char **argv)
{
	std::tuple<std::string, std::string, std::string> tup("Geeks","for","Geeks");
	std::map<uint32_t, uint32_t> index;
	index[0] = 1;
	index[2] = 1;
	printTuple(tup, index);



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
