#include "logic/init_service/component_name.h"

namespace apie {


Component_Name::Component_Name(uint64_t accountId)
	: m_accountId(accountId)
{

}

void Component_Name::loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader)
{
	m_server = server;

	if (ptrLoader->has<Single_Name_Loader>())
	{
		auto& optData = ptrLoader->lookup<Single_Name_Loader>().getData();
		m_dbData = optData;

		if (m_dbData.has_value())
		{
			m_dbData.value().SetDbProxyServer(m_server);
		}
	}
}

void Component_Name::loadFromDbDone()
{
}

void Component_Name::saveToDb()
{
	std::time_t t = std::time(nullptr);
	std::tm tm = *std::localtime(&t);
	std::stringstream ss;
	ss << std::put_time(&tm, "%c");

	if (m_dbData.has_value())
	{
		m_dbData.value().set_name(ss.str());
		m_dbData.value().Update();
	}
}

void Component_Name::initCreate(DoneFunctor functorObj)
{
	if (m_dbData.has_value())
	{
		auto cb = [functorObj](apie::status::Status status, bool result, uint64_t affectedRows) {
			if (!status.ok())
			{
				functorObj(false);
			}
			else
			{
				functorObj(true);
			}
		};

		std::time_t t = std::time(nullptr);
		std::tm tm = *std::localtime(&t);
		std::stringstream ss;
		ss << std::put_time(&tm, "%c");

		m_dbData.value().set_name(ss.str());
		m_dbData.value().Update(cb);
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
		dbObj.SetDbProxyServer(m_server);
		dbObj.Insert(cb);

		//InsertToDb<apie::dbt_account::account_name_AutoGen>(m_server, dbObj, cb);
	}
}

}

