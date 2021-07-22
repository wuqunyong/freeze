#include "service_init.h"

#include "logic/dbproxy_mgr.h"

namespace apie {

apie::status::Status initHook()
{
	return DBProxyMgrSingleton::get().init();
}

apie::status::Status startHook()
{
	return DBProxyMgrSingleton::get().start();
}

apie::status::Status readyHook()
{
	return DBProxyMgrSingleton::get().ready();
}

apie::status::Status exitHook()
{
	DBProxyMgrSingleton::get().exit();
	return { apie::status::StatusCode::OK, "" };
}

}

