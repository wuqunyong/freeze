#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "json/json.h"

#include "apie.h"
#include "logic/mock_role.h"

namespace apie {


class RoleEchoModule
{
public:
	static void registerModule();

	static void onEcho(MockRole& mockRole, ::pubsub::TEST_CMD& msg);
	static void onAsyncEcho(MockRole& mockRole, ::pubsub::TEST_CMD& msg);
	
public:
	static inline const std::string s_sName = "echo";
};


}
