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


namespace apie {

	template <typename T>
	struct ComponentWrapper
	{
		using Type = T;
	};

	class Module_Create;
	class Module_Name;


	class AccountFactory
	{
	public:
		using PrimaryKey = uint64_t;
		using ComponentWrapperTuple = std::tuple<ComponentWrapper<Module_Create>, ComponentWrapper<Module_Name>>;

		using AccountLoader = ComponentLoader<PrimaryKey, ComponentWrapperTuple>;
		using AccountLoaderPtr = std::shared_ptr<ComponentLoader<PrimaryKey, ComponentWrapperTuple>>;
		using Callback = std::function<void(apie::status::Status status, AccountLoaderPtr)>;


		static AccountLoaderPtr CreateAccount(PrimaryKey iId);
		static void LoadAccountFromDb(PrimaryKey iId, Callback cb);

		static void AddAccount(AccountLoaderPtr ptrLoader);
		static AccountLoaderPtr FindAccount(PrimaryKey iId);

	private:
		static inline std::map<PrimaryKey, AccountLoaderPtr> m_accounts;
	};

	template <typename ComponentType>
	using UnwrapComponentWrapper = ComponentWrapper<ComponentType>::Type;
}
