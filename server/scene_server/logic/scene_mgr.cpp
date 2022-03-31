#include "logic/scene_mgr.h"

#include "../../common/opcodes.h"

#include "module_handler/scene_mgr_module.h"

namespace apie {

std::string SceneMgr::moduleName()
{
	return "SceneMgr";
}

uint32_t SceneMgr::modulePrecedence()
{
	return 1;
}

SceneMgr::SceneMgr(std::string name, module_loader::ModuleLoaderBase* prtLoader)
	: m_name(name),
	m_prtLoader(prtLoader)
{

}

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

apie::status::Status SceneMgr::exit()
{
	return { apie::status::StatusCode::OK, "" };
}


}

