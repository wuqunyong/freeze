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


namespace apie {

	struct Multi_Account_Loader
	{
		using Type = MultiRowLoader<apie::dbt_account::account_AutoGen>;
	};

	class AccountLoader;

	class Component_Create
	{
	public:
		using DoneFunctor = std::function<void(bool)>;

		Component_Create(uint64_t accountId = 0);

		void loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader);
		void loadFromDbDone();
		void saveToDb();
		void initCreate(DoneFunctor cbObj);

		void TestFunc();

	private:
		uint64_t m_accountId;

		::rpc_msg::CHANNEL m_server;
		std::map<uint64_t, apie::dbt_account::account_AutoGen> m_account;
	};
}
