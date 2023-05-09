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
	class Component_Name;


	class AccountLoader
	{
	public:
		using PrimaryKey = uint64_t;
		using ComponentWrapperTuple = std::tuple<ComponentWrapper<Module_Create>, ComponentWrapper<Component_Name>>;

		using Loader = ComponentLoader<PrimaryKey, ComponentWrapperTuple>;

		using LoaderPtr = std::shared_ptr<Loader>;
		using Callback = std::function<void(apie::status::Status status, LoaderPtr)>;


		static LoaderPtr Create(PrimaryKey iId);
		static void LoadFromDb(PrimaryKey iId, Callback cb);

		static void Add(LoaderPtr ptrLoader);
		static LoaderPtr Find(PrimaryKey iId);
		static void Erase(PrimaryKey iId);

	private:
		static inline std::map<PrimaryKey, LoaderPtr> m_accounts;
	};


	template <typename ComponentType>
	using UnwrapComponentWrapper = ComponentWrapper<ComponentType>::Type;
}
