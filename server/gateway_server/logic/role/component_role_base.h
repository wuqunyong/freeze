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

#include "common/dao/init.h"
#include "logic/role/role.h"

namespace apie {

	struct Single_RoleBase_Loader
	{
		using Type = SingleRowLoader<apie::dbt_role::role_base_AutoGen>;
	};

	class RoleLoader;

	class Component_RoleBase
	{
	public:
		using DoneFunctor = std::function<void(bool)>;

		Component_RoleBase(uint64_t roleId = 0);

		void loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader);
		void loadFromDbDone();
		void saveToDb();
		void initCreate(DoneFunctor functorObj);

		void addLevel();

	private:
		uint64_t m_roleId;

		::rpc_msg::CHANNEL m_server;
		std::optional<apie::dbt_role::role_base_AutoGen> m_dbData;
	};
}
