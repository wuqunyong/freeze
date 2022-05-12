#include "role_login_module.h"

#include "../../common/opcodes.h"

namespace apie {


void RoleLoginModule::registerModule()
{
	MockRole::addHandler(s_sName, "login", std::bind(&RoleLoginModule::onLogin, std::placeholders::_1, std::placeholders::_2));
}

void RoleLoginModule::onLogin(MockRole& mockRole, ::pubsub::TEST_CMD& msg)
{
	pb::login::LoginC2LS request;
	request.set_game_id(1);
	request.set_user_id(mockRole.getIggId());
	request.set_version(1);
	mockRole.sendMsg(MergeOpcode(::apie::_MSG_CLIENT_LOGINTOL, 0), request);

	mockRole.addPendingResponse(MergeOpcode(::apie::_MSG_GAMESERVER_LOGINRESP, 0), MergeOpcode(::apie::_MSG_CLIENT_LOGINTOL, 0));
}

}

