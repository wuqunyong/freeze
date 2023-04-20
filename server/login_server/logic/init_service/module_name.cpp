#include "logic/init_service/module_name.h"

namespace apie {


Module_Name::Module_Name(uint64_t accountId)
	: m_accountId(accountId)
{

}

void Module_Name::loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader)
{
	m_server = server;

	if (ptrLoader->has<Single_Name_Loader>())
	{
		auto& optData = ptrLoader->lookup<Single_Name_Loader>().getData();
		m_dbData = optData;
	}
}

void Module_Name::loadFromDbDone()
{
}

void Module_Name::saveToDb()
{

}

void Module_Name::initCreate(DoneFunctor functorObj)
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
		ss << std::put_time(&tm, "%c");

		dbObj.set_name(ss.str());

		InsertToDb<apie::dbt_account::account_name_AutoGen>(m_server, dbObj, cb);
	}
}

}

