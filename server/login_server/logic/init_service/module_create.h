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

	class Account;

	class Module_Create
	{
	public:
		Module_Create(uint64_t accountId = 0);

		void loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader);
		void loadFromDbDone();
		void saveToDb();
		void initCreate(auto cbObj)
		{
			if (!m_account.empty())
			{
				cbObj(true);
			}
			else
			{
				auto cb = [cbObj](apie::status::Status status, bool result, uint64_t affectedRows, uint64_t insertId) {
					if (!status.ok())
					{
						cbObj(false);
					}
					else
					{
						cbObj(true);
					}
				};
				apie::dbt_account::account_AutoGen dbObj(m_accountId);
				dbObj.set_register_time(time(nullptr));

				InsertToDb<apie::dbt_account::account_AutoGen>(m_server, dbObj, cb);
			}
		}

	private:
		uint64_t m_accountId;

		::rpc_msg::CHANNEL m_server;
		std::map<uint64_t, apie::dbt_account::account_AutoGen> m_account;
	};
}
