#include "scene_mgr.h"

#include "../../common/opcodes.h"

#include "../module_handler/scene_mgr_module.h"

namespace apie {

apie::status::Status SceneMgr::init()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_Scene_Server });
	if (!bResult)
	{
		return {apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}

	SceneMgrModule::init();

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status SceneMgr::start()
{
	apie::hook::HookRegistrySingleton::get().triggerHook(hook::HookPoint::HP_Ready);

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status SceneMgr::ready()
{
	SceneMgrModule::ready();

	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ServerStatus", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

void SceneMgr::exit()
{

}


}

