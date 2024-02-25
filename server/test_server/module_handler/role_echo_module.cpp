#include "role_echo_module.h"


namespace apie {


void RoleEchoModule::registerModule()
{
	MockRole::addHandler(s_sName, "sync", std::bind(&RoleEchoModule::onEcho, std::placeholders::_1, std::placeholders::_2));
	MockRole::addHandler(s_sName, "async", std::bind(&RoleEchoModule::onAsyncEcho, std::placeholders::_1, std::placeholders::_2));
}

void RoleEchoModule::onEcho(MockRole& mockRole, ::pubsub::TEST_CMD& msg)
{
	::login_msg::EchoRequest request;
	request.set_value1(std::stoull(msg.params()[0]));
	request.set_value2(msg.params()[1]);
	mockRole.sendMsg(pb::core::OP_EchoRequest, request);
}

void RoleEchoModule::onAsyncEcho(MockRole& mockRole, ::pubsub::TEST_CMD& msg)
{
	::login_msg::AsyncEchoRequest request;
	request.set_value1(std::stoull(msg.params()[0]));
	request.set_value2(msg.params()[1]);
	mockRole.sendMsg(pb::core::OP_AsyncEchoRequest, request);
}

}

