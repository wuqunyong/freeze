#include "role_login_module.h"


namespace apie {


void RoleLoginModule::registerModule()
{
	MockRole::addHandler(s_sName, "login", std::bind(&RoleLoginModule::onLogin, std::placeholders::_1, std::placeholders::_2));
}

void RoleLoginModule::onLogin(MockRole& mockRole, ::pubsub::TEST_CMD& msg)
{
	::login_msg::AccountLoginRequest request;
	request.set_account_id(mockRole.getIggId());
	mockRole.sendMsg(pb::core::OP_AccountLoginRequest, request);
}

}

