#include "logic/role/role.h"

#include "logic/role/component_role_base.h"

namespace apie {


RoleLoader::LoaderPtr RoleLoader::Create(PrimaryKey iId)
{
	static ComponentWrapperTuple kTuple;

	auto pInstance = MakeComponentLoader(iId, kTuple);
	return pInstance;
}

void RoleLoader::LoadFromDb(PrimaryKey iId, Callback doneCb)
{
	auto ptrModuleLoader = RoleLoader::Create(iId);

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DbRole_Proxy);
	server.set_id(1);


	auto ptrLoad = CreateDBLoaderPtr();

	ptrLoad->set<Single_RoleBase_Loader>(iId);

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

