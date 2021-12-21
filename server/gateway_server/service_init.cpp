#include "service_init.h"

#include "logic/gateway_mgr.h"

namespace apie {

apie::status::Status initHook()
{
	return GatewayMgrSingleton::get().init();
}

apie::status::Status startHook()
{
	return GatewayMgrSingleton::get().start();
}

apie::status::Status readyHook()
{
	return GatewayMgrSingleton::get().ready();
}

apie::status::Status exitHook()
{
	GatewayMgrSingleton::get().exit();
	return { apie::status::StatusCode::OK, "" };
}

}

