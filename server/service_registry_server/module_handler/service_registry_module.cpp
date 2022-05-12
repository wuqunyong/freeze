#include "module_handler/service_registry_module.h"

#include "logic/service_registry.h"

namespace apie {

void ServiceRegistryModule::init()
{
	// PUBSUB
	auto& pubsub = apie::pubsub::PubSubManagerSingleton::get();
	pubsub.subscribe<::pubsub::LOGIC_CMD>(::pubsub::PT_LogicCmd, ServiceRegistryModule::PubSub_logicCmd);
	pubsub.subscribe<::pubsub::SERVER_PEER_CLOSE>(::pubsub::PT_ServerPeerClose, ServiceRegistryModule::PubSub_serverPeerClose);

	// CMD
	auto& cmd = LogicCmdHandlerSingleton::get();
	cmd.init();
	cmd.registerOnCmd("provider", "show_provider", ServiceRegistryModule::Cmd_showProvider);

	// Inner Protocols		
	using namespace ::service_discovery;
	S_INTRA_REGISTER_SERVICE(REGISTER_INSTANCE, ServiceRegistryModule::handleRequestRegisterInstance);
	S_INTRA_REGISTER_SERVICE(HEARTBEAT, ServiceRegistryModule::handleRequestHeartbeat);
}

void ServiceRegistryModule::ready()
{
}

void ServiceRegistryModule::Cmd_showProvider(::pubsub::LOGIC_CMD& cmd)
{
	std::stringstream ss;
	ss << std::endl;
	for (const auto& items : APieGetModule<apie::ServiceRegistry>()->registered())
	{
		ss << "--> " << "addTime:" << items.second.addTime << "|modifiedTime:" << items.second.modifyTime << "|node:" << items.second.instance.ShortDebugString() << std::endl;
	}

	ASYNC_PIE_LOG("ServiceRegistryModule/show_provider", PIE_CYCLE_DAY, PIE_NOTICE, "%s", ss.str().c_str());

}

void ServiceRegistryModule::PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{
	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

apie::status::Status  ServiceRegistryModule::handleRequestRegisterInstance(MessageInfo info, const std::shared_ptr<::service_discovery::MSG_REQUEST_REGISTER_INSTANCE>& request,
	std::shared_ptr<::service_discovery::MSG_RESPONSE_REGISTER_INSTANCE>& response)
{
	std::stringstream ss;
	ss << "iSerialNum:" << info.iSessionId << ",request:" << request->ShortDebugString();

	auto auth = apie::CtxSingleton::get().identify().auth;
	if (!auth.empty() && auth != request->auth())
	{
		response->set_status_code(opcodes::SC_Discovery_AuthError);

		ss << ",auth:error";
		ASYNC_PIE_LOG("SelfRegistration/handleRequestRegisterInstance", PIE_CYCLE_DAY, PIE_ERROR, ss.str().c_str());
		return { apie::status::StatusCode::OK, "" };
	}

	EndPoint addNode(request->instance().realm(), request->instance().type(), request->instance().id(), "");
	auto nodeOpt = APieGetModule<apie::ServiceRegistry>()->findNode(addNode);
	if (!nodeOpt.has_value())
	{
		response->set_status_code(opcodes::SC_Discovery_InvalidPoint);

		ss << ",invalid node";
		ASYNC_PIE_LOG("SelfRegistration/handleRequestRegisterInstance", PIE_CYCLE_DAY, PIE_ERROR, ss.str().c_str());
		return { apie::status::StatusCode::OK, "" };
	}

	::service_discovery::EndPointInstance instanceObj = request->instance();
	instanceObj.set_ip(nodeOpt.value().fields.ip);
	instanceObj.set_port(nodeOpt.value().fields.port);

	bool bResult = APieGetModule<apie::ServiceRegistry>()->updateInstance(info.iSessionId, instanceObj);
	if (!bResult)
	{
		response->set_status_code(opcodes::SC_Discovery_DuplicateNode);

		ss << ",node:duplicate";
		ASYNC_PIE_LOG("SelfRegistration/handleRequestRegisterInstance", PIE_CYCLE_DAY, PIE_ERROR, ss.str().c_str());
		return { apie::status::StatusCode::OK, "" };
	}
	
	ASYNC_PIE_LOG("SelfRegistration/handleRequestRegisterInstance", PIE_CYCLE_DAY, PIE_DEBUG, ss.str().c_str());

	response->set_status_code(::opcodes::StatusCode::SC_Ok);
	response->set_listeners_config(nodeOpt.value().fields.listeners_config);
	response->set_mysql_config(nodeOpt.value().fields.mysql_config);
	response->set_nats_config(nodeOpt.value().fields.nats_config);
	response->set_redis_config(nodeOpt.value().fields.redis_config);

	auto cb = [](){
		APieGetModule<apie::ServiceRegistry>()->broadcast();
	};
	apie::CtxSingleton::get().getLogicThread()->dispatcher().post(cb);

	return { apie::status::StatusCode::OK, "" };
}


apie::status::Status  ServiceRegistryModule::handleRequestHeartbeat(MessageInfo info, const std::shared_ptr<::service_discovery::MSG_REQUEST_HEARTBEAT>& request,
		std::shared_ptr<::service_discovery::MSG_RESPONSE_HEARTBEAT>& response)
{
	std::stringstream ss;
	ss << "iSerialNum:" << info.iSessionId << ",request:" << request->ShortDebugString();

	response->set_status_code(opcodes::SC_Ok);

	bool bResult = APieGetModule<apie::ServiceRegistry>()->updateHeartbeat(info.iSessionId);
	if (!bResult)
	{
		response->set_status_code(opcodes::SC_Discovery_Unregistered);

		ss << "node:Unregistered";
		ASYNC_PIE_LOG("SelfRegistration/handleRequestHeartbeat", PIE_CYCLE_DAY, PIE_ERROR, ss.str().c_str());
		return { apie::status::StatusCode::OK, "" };
	}

	return { apie::status::StatusCode::OK, "" };
}


void ServiceRegistryModule::PubSub_serverPeerClose(const std::shared_ptr<::pubsub::SERVER_PEER_CLOSE>& msg)
{
	std::stringstream ss;
	ss << "topic:"<< ",refMsg:" << msg->ShortDebugString();
	ASYNC_PIE_LOG("SelfRegistration/onServerPeerClose", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	uint64_t iSerialNum = msg->serial_num();
	bool bChanged = APieGetModule<apie::ServiceRegistry>()->deleteBySerialNum(iSerialNum);
	if (bChanged)
	{
		APieGetModule<apie::ServiceRegistry>()->broadcast();
	}
}

}

