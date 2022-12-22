#include "logic/login_mgr.h"

#include "../../common/dao/init.h"
#include "../../common/opcodes.h"

#include "module_handler/login_mgr_module.h"


namespace apie {

std::string LoginMgr::moduleName()
{
	return "LoginMgr";
}

uint32_t LoginMgr::modulePrecedence()
{
	return 1;
}

LoginMgr::LoginMgr(std::string name, module_loader::ModuleLoaderBase* prtLoader)
	: m_name(name),
	m_prtLoader(prtLoader)
{

}

apie::status::Status LoginMgr::init()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_Login_Server });
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}

	LoginMgrModule::init();

	return {apie::status::StatusCode::OK, ""};
}

apie::status::Status LoginMgr::start()
{
	return DoBindTables<LoginMgr>(this, hook::HookPoint::HP_Start, apie::Ctx::getThisChannel().realm(), apie::CtxSingleton::get().getConfigs()->bind_tables);
}

apie::status::Status LoginMgr::load()
{
	apie::dbt_account::account_name_AutoGen user(1);

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ACCOUNT_Proxy);
	server.set_id(1);

	auto cb = [](status::Status status, std::vector<apie::dbt_account::account_name_AutoGen>& userList) {
		if (!status.ok())
		{
			return;
		}
	};
	LoadFromDbByQueryAll<apie::dbt_account::account_name_AutoGen>(server, user, cb);

	return { apie::status::StatusCode::OK_ASYNC, "" };
}

apie::status::Status LoginMgr::ready()
{
	LoginMgrModule::ready();

	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ServerStatus", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status LoginMgr::exit()
{
	return { apie::status::StatusCode::OK, "" };
}

void LoginMgr::setHookReady(hook::HookPoint point)
{
	m_prtLoader->setHookReady(point);
}

}

