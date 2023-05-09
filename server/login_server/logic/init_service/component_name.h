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

#include "../../../common/dao/init.h"
#include "logic/init_service/account.h"

namespace apie {

	struct Single_Name_Loader
	{
		using Type = SingleRowLoader<apie::dbt_account::account_name_AutoGen>;
	};

	class AccountLoader;

	class Component_Name
	{
	public:
		using DoneFunctor = std::function<void(bool)>;

		Component_Name(uint64_t accountId = 0);

		void loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader);
		void loadFromDbDone();
		void saveToDb();
		void initCreate(DoneFunctor functorObj);

	private:
		uint64_t m_accountId;

		::rpc_msg::CHANNEL m_server;
		std::optional<apie::dbt_account::account_name_AutoGen> m_dbData;
	};
}
