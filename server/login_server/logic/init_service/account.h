#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"
#include "pb_init.h"

#include "logic/init_service/module_create.h"

namespace apie {

	template <typename T>
	struct ModuleWrapper
	{
		using Type = T;;
	};

	class Account
	{
	public:
		using PrimaryKey = uint64_t;
		using ModuleTuple = std::tuple<ModuleWrapper<Module_Create>>;
		using AccountPtr = std::shared_ptr<CommonModuleLoader<PrimaryKey, ModuleTuple>>;
		using Callback = std::function<void(apie::status::Status status, AccountPtr)>;


		static AccountPtr CreateAccount(PrimaryKey iId)
		{
			static ModuleTuple kModuleTuple;

			auto pInstance = MakeCommonModuleLoader(iId, kModuleTuple);
			return pInstance;
		}

		static void LoadAccountFromDb(PrimaryKey iId, Callback cb);


		Account(uint64_t accountId);



	private:
		uint64_t m_accountId;
	};
}
