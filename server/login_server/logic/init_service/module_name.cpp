#include "logic/init_service/module_name.h"

namespace apie {


Module_Name::Module_Name(uint64_t accountId)
	: m_accountId(accountId)
{

}

void Module_Name::loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader)
{
	m_server = server;
}

void Module_Name::loadFromDbDone()
{
}

void Module_Name::saveToDb()
{

}


}

