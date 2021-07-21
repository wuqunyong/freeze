#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"

namespace apie {

	class SceneMgr
	{
	public:
		apie::status::Status init();
		apie::status::Status start();
		apie::status::Status ready();
		void exit();

	private:

	};

	using SceneMgrSingleton = ThreadSafeSingleton<SceneMgr>;
}
