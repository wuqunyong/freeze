#include "logic/account/component_create.h"

namespace apie {


Component_Create::Component_Create(uint64_t accountId)
	: m_accountId(accountId)
{

}

void Component_Create::loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader)
{
	m_server = server;

	if (ptrLoader->has<Multi_Account_Loader>())
	{
		auto& vecData = ptrLoader->lookup<Multi_Account_Loader>().getData();
		for (const auto& elems : vecData)
		{
			auto iId = elems.get_account_id();
			m_account[iId] = elems;
		}
	}
}

void Component_Create::loadFromDbDone()
{
}

void Component_Create::saveToDb()
{

}

void Component_Create::initCreate(DoneFunctor cbObj)
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

void Component_Create::TestFunc()
{

}

}

