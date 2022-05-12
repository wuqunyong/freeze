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


class RoleTalentModule
{
public:
	static void registerModule();

	static void onOpen(MockRole& mockRole, ::pubsub::TEST_CMD& msg);
	static void onChoose(MockRole& mockRole, ::pubsub::TEST_CMD& msg);
	static void onActivate(MockRole& mockRole, ::pubsub::TEST_CMD& msg);

public:
	static inline const std::string s_sName = "talent";
};


}
