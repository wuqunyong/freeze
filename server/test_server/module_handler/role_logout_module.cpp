#include "role_logout_module.h"

#include "logic/test_server.h"

namespace apie {


void RoleLogoutModule::registerModule()
{
	MockRole::addHandler(s_sName, "logout", std::bind(&RoleLogoutModule::onLogout, std::placeholders::_1, std::placeholders::_2));
}

void RoleLogoutModule::onLogout(MockRole& mockRole, ::pubsub::TEST_CMD& msg)
{
	APieGetModule<apie::TestServerMgr>()->removeMockRole(mockRole.getIggId());
}

}

