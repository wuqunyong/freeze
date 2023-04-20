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

		using Loader = ComponentLoader<PrimaryKey, ComponentWrapperTuple>;
		using LoaderPtr = std::shared_ptr<ComponentLoader<PrimaryKey, ComponentWrapperTuple>>;
		using Callback = std::function<void(apie::status::Status status, LoaderPtr)>;


		static LoaderPtr CreateAccount(PrimaryKey iId);
		static void LoadAccountFromDb(PrimaryKey iId, Callback cb);

		static void AddAccount(LoaderPtr ptrLoader);
		static LoaderPtr FindAccount(PrimaryKey iId);

	private:
		static inline std::map<PrimaryKey, LoaderPtr> m_accounts;
	};


	template <typename ComponentType>
	using UnwrapComponentWrapper = ComponentWrapper<ComponentType>::Type;
}
