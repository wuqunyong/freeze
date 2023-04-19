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

	struct Single_Name_Loader
	{
		using Type = SingleRowLoader<apie::dbt_account::account_name_AutoGen>;
	};

	class AccountFactory;

	class Module_Name
	{
	public:
		Module_Name(uint64_t accountId = 0);

		void loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader);
		void loadFromDbDone();
		void saveToDb();
		void initCreate(auto functorObj);

	private:
		uint64_t m_accountId;

		::rpc_msg::CHANNEL m_server;
		std::optional<apie::dbt_account::account_name_AutoGen> m_dbData;
	};

	void Module_Name::initCreate(auto functorObj)
	{
		if (m_dbData.has_value())
		{
			functorObj(true);
		}
		else
		{
			auto cb = [functorObj](apie::status::Status status, bool result, uint64_t affectedRows, uint64_t insertId) {
				if (!status.ok())
				{
					functorObj(false);
				}
				else
				{
					functorObj(true);
				}
			};
			apie::dbt_account::account_name_AutoGen dbObj(m_accountId);

			std::time_t t = std::time(nullptr);
			std::tm tm = *std::localtime(&t);
			std::stringstream ss;
			ss << std::put_time(&tm, "%c %Z");

			dbObj.set_name(ss.str());

			InsertToDb<apie::dbt_account::account_name_AutoGen>(m_server, dbObj, cb);
		}
	}
}
