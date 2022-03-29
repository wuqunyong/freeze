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


class DBProxyMgr
{
public:
	DBProxyMgr(std::string name, module_loader::ModuleLoaderBase* prtLoader);

	static std::string moduleName();
	static uint32_t modulePrecedence();

	apie::status::Status init();
	apie::status::Status start();
	apie::status::Status ready();
	apie::status::Status exit();

private:
	std::string m_name;
	module_loader::ModuleLoaderBase* m_prtLoader;
};


}
