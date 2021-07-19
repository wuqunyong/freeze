#include "service_init.h"

#include "logic/login_mgr.h"

namespace APie {

apie::status::Status initHook()
{
	return LoginMgrSingleton::get().init();
}

apie::status::Status startHook()
{
	return LoginMgrSingleton::get().start();
}

apie::status::Status readyHook()
{
	return LoginMgrSingleton::get().ready();
}

apie::status::Status exitHook()
{
	LoginMgrSingleton::get().exit();
	return {apie::status::StatusCode::OK, ""};
}

}

