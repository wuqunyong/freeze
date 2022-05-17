#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>
#include <memory>
#include <set>

#include "apie.h"
#include "../../pb_msg/init.h"

namespace apie {

	class MockRole;

	struct TalentData
	{
		static void handleLoginNotice(MockRole* ptrRole, MessageInfo info, const std::string& msg);

		pb::talent::TalentPanel panle;
	};


}
