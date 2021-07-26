#include "service_init.h"

#include "logic/test_server.h"

namespace apie {

apie::status::Status initHook()
{
	return TestServerMgrSingleton::get().init();
}

apie::status::Status startHook()
{
	return TestServerMgrSingleton::get().start();
}

apie::status::Status readyHook()
{
	return TestServerMgrSingleton::get().ready();
}

apie::status::Status exitHook()
{
	TestServerMgrSingleton::get().exit();
	return { apie::status::StatusCode::OK, "" };
}

}

