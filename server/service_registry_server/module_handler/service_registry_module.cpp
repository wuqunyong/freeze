#include "module_handler/service_registry_module.h"

#include "logic/service_registry.h"

namespace apie {

void ServiceRegistryModule::init()
{
	// PUBSUB
	auto& pubsub = apie::pubsub::PubSubManagerSingleton::get();
	pubsub.subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, ServiceRegistryModule::PubSub_logicCmd);
	pubsub.subscribe<::pubsub::SERVER_PEER_CLOSE>(::pubsub::PT_ServerPeerClose, ServiceRegistryModule::PubSub_serverPeerClose);

	// CMD
	auto& cmd = LogicCmdHandlerSingleton::get();
	cmd.init();
	cmd.registerOnCmd("provider", "show_provider", ServiceRegistryModule::Cmd_showProvider);

	// Inner Protocols		
	auto& server = apie::service::ServiceHandlerSingleton::get().server;
	server.createService<::service_discovery::MSG_REQUEST_REGISTER_INSTANCE, opcodes::OP_DISCOVERY_MSG_RESP_REGISTER_INSTANCE, ::service_discovery::MSG_RESP_REGISTER_INSTANCE>(
		::opcodes::OP_DISCOVERY_MSG_REQUEST_REGISTER_INSTANCE, ServiceRegistryModule::handleRequestRegisterInstance);
	server.createService<::service_discovery::MSG_REQUEST_HEARTBEAT, opcodes::OP_DISCOVERY_MSG_RESP_HEARTBEAT, ::service_discovery::MSG_RESP_HEARTBEAT>(
		::opcodes::OP_DISCOVERY_MSG_REQUEST_HEARTBEAT, ServiceRegistryModule::handleRequestHeartbeat);
}

void ServiceRegistryModule::ready()
{
}

void ServiceRegistryModule::Cmd_showProvider(::pubsub::LOGIC_CMD& cmd)
{
	std::stringstream ss;
	ss << std::endl;
	for (const auto& items : ServiceRegistrySingleton::get().registered())
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

apie::status::Status  ServiceRegistryModule::handleRequestRegisterInstance(uint64_t iSerialNum, const std::shared_ptr<::service_discovery::MSG_REQUEST_REGISTER_INSTANCE>& request,
	std::shared_ptr<::service_discovery::MSG_RESP_REGISTER_INSTANCE>& response)
{
	std::stringstream ss;
	ss << "iSerialNum:" << iSerialNum << ",request:" << request->ShortDebugString();

	auto auth = apie::CtxSingleton::get().identify().auth;
	if (!auth.empty() && auth != request->auth())
	{
		response->set_status_code(opcodes::SC_Discovery_AuthError);

		ss << ",auth:error";
		ASYNC_PIE_LOG("SelfRegistration/handleRequestRegisterInstance", PIE_CYCLE_DAY, PIE_ERROR, ss.str().c_str());
		return { apie::status::StatusCode::OK, "" };
	}

	bool bResult = ServiceRegistrySingleton::get().updateInstance(iSerialNum, request->instance());
	if (!bResult)
	{
		response->set_status_code(opcodes::SC_Discovery_DuplicateNode);

		ss << ",node:duplicate";
		ASYNC_PIE_LOG("SelfRegistration/handleRequestRegisterInstance", PIE_CYCLE_DAY, PIE_ERROR, ss.str().c_str());
		return { apie::status::StatusCode::OK, "" };
	}
	
	ASYNC_PIE_LOG("SelfRegistration/handleRequestRegisterInstance", PIE_CYCLE_DAY, PIE_DEBUG, ss.str().c_str());

	response->set_status_code(::opcodes::StatusCode::SC_Ok);

	auto cb = [](){
		ServiceRegistrySingleton::get().broadcast();
	};
	apie::CtxSingleton::get().getLogicThread()->dispatcher().post(cb);

	return { apie::status::StatusCode::OK, "" };
}


apie::status::Status  ServiceRegistryModule::handleRequestHeartbeat(uint64_t iSerialNum, const std::shared_ptr<::service_discovery::MSG_REQUEST_HEARTBEAT>& request,
		std::shared_ptr<::service_discovery::MSG_RESP_HEARTBEAT>& response)
{
	std::stringstream ss;
	ss << "iSerialNum:" << iSerialNum << ",request:" << request->ShortDebugString();

	response->set_status_code(opcodes::SC_Ok);

	bool bResult = ServiceRegistrySingleton::get().updateHeartbeat(iSerialNum);
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
	bool bChanged = ServiceRegistrySingleton::get().deleteBySerialNum(iSerialNum);
	if (bChanged)
	{
		ServiceRegistrySingleton::get().broadcast();
	}
}

}

