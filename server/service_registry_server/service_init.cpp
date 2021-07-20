#include "service_init.h"

#include "logic/service_registry.h"

namespace apie {

apie::status::Status initHook()
{
	return ServiceRegistrySingleton::get().init();
}

apie::status::Status startHook()
{
	return ServiceRegistrySingleton::get().start();
}

apie::status::Status readyHook()
{
	return ServiceRegistrySingleton::get().ready();
}

apie::status::Status exitHook()
{
	ServiceRegistrySingleton::get().exit();
	return { apie::status::StatusCode::OK, "" };
}

}

