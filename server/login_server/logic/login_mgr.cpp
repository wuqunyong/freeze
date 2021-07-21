#include "login_mgr.h"

#include "../../common/dao/model_account.h"
#include "../../common/dao/model_account_name.h"
#include "../../common/opcodes.h"

#include "../module_handler/login_mgr_module.h"


namespace apie {

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
	apie::hook::HookRegistrySingleton::get().triggerHook(hook::HookPoint::HP_Ready);
	return { apie::status::StatusCode::OK, "" };
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

void LoginMgr::exit()
{

}


}

