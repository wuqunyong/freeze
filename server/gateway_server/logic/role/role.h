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

	class Component_RoleBase;


	class RoleLoader
	{
	public:
		using PrimaryKey = uint64_t;
		using ComponentWrapperTuple = std::tuple<ComponentWrapper<Component_RoleBase>>;

		using Loader = ComponentLoader<PrimaryKey, ComponentWrapperTuple>;

		using LoaderPtr = std::shared_ptr<Loader>;
		using Callback = std::function<void(apie::status::Status status, LoaderPtr)>;


		static LoaderPtr Create(PrimaryKey iId);
		static void LoadFromDb(PrimaryKey iId, Callback cb);

	};

}
