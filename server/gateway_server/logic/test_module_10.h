#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"

#include "../../common/dao/model_user.h"
#include "../../common/dao/model_account.h"
#include "../../common/dao/model_role_extra.h"
#include "../../common/dao/varchars1_AutoGen.h"

namespace apie {


	struct Single_ModelUser_Loader
	{
		using Type = SingleRowLoader<apie::ModelUser>;
	};

	struct Single_ModelRoleExtra_Loader
	{
		using Type = SingleRowLoader<apie::ModelRoleExtra>;
	};

	struct Single_ModelVarchars1_Loader
	{
		using Type = SingleRowLoader<apie::varchars1_AutoGen>;
	};


	struct Multi_ModelUser_Loader
	{
		using Type = MultiRowLoader<apie::ModelUser>;
	};

	struct Single_ModelAccount_Loader
	{
		using Type = SingleRowLoader<apie::ModelAccount>;
	};

	class TestModule10
	{
	public:
		TestModule10(std::string name, module_loader::ModuleLoaderBase* prtLoader);

		static std::string moduleName();
		static uint32_t modulePrecedence();

		apie::status::Status init();
		apie::status::Status start();
		apie::status::Status ready();
		apie::status::Status exit();

		void setHookReady(hook::HookPoint point);

	private:
		std::string m_name;
		module_loader::ModuleLoaderBase* m_prtLoader;
	};
}
