#include "logic/init_service/account.h"

namespace apie {


Account::Account(uint64_t accountId)
	: m_accountId(accountId)
{

}

void Account::LoadAccountFromDb(PrimaryKey iId, Callback doneCb)
{
	auto ptrModuleLoader = Account::CreateAccount(iId);

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DbAccount_Proxy);
	server.set_id(1);


	auto ptrLoad = CreateDBLoaderPtr();

	ptrLoad->set<Multi_Account_Loader>(iId).lookup<Multi_Account_Loader>().getTableType().set_account_id(iId);
	ptrLoad->set<Multi_Account_Loader>(iId).lookup<Multi_Account_Loader>().markFilter({ apie::dbt_account::account_AutoGen::account_id });

	auto cb = [ptrModuleLoader, doneCb, server](apie::status::Status status, std::shared_ptr<apie::DbLoadComponent> loader) {
		if (!status.ok())
		{
			doneCb(status, ptrModuleLoader);
			return;
		}

		ptrModuleLoader->Meta_loadFromDbLoader(server, loader);
		ptrModuleLoader->Meta_loadFromDbDone();
		
		auto cb = [doneCb](apie::status::Status status, auto ptrLoader) {
			doneCb(status, ptrLoader);
		};
		ptrModuleLoader->Meta_initCreate(cb);
	};
	ptrLoad->loadFromDb(server, cb);
}

}

