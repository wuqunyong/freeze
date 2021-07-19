#include "service_init.h"

#include "apie.h"
#include "logic/scene_mgr.h"

namespace APie {

apie::status::Status initHook()
{
	return SceneMgrSingleton::get().init();
}

apie::status::Status startHook()
{
	return SceneMgrSingleton::get().start();
}

apie::status::Status readyHook()
{
	return SceneMgrSingleton::get().ready();
}

apie::status::Status exitHook()
{
	SceneMgrSingleton::get().exit();
	return { apie::status::StatusCode::OK, "" };
}

}

