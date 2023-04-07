#include "logic/init_service/module_create.h"

namespace apie {


Module_Create::Module_Create(uint64_t accountId)
	: m_accountId(accountId)
{

}

void Module_Create::loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader)
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

void Module_Create::loadFromDbDone()
{
}

void Module_Create::saveToDb()
{

}


}

