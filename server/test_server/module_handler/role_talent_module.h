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

public:
	static inline const std::string s_sName = "talent";
};


}
