#include "apie/forward/forward_manager.h"

#include "apie/common/protobuf_factory.h"
#include "apie/network/logger.h"

namespace apie {
namespace forward {


ForwardManager::ForwardManager()
{
	this->init();
}

ForwardManager::~ForwardManager()
{
	this->destroy();
}

std::optional<std::string> ForwardManager::getType(uint32_t opcode)
{
	auto find_ite = type_.find(opcode);
	if (find_ite == type_.end())
	{
		return std::nullopt;
	}

	return find_ite->second;
}

void ForwardManager::init()
{

}

void ForwardManager::destroy()
{
	type_.clear();
	func_.clear();
	service_.clear();
}

bool ForwardManager::sendForwardMux(const ::rpc_msg::CHANNEL& server, const ::rpc_msg::RoleIdentifier& role, MessageInfo info, const std::string& msg)
{
	::rpc_msg::RPC_Multiplexer_Forward mux;
	*mux.mutable_role() = role;
	mux.mutable_info()->set_session_id(info.iSessionId);
	mux.mutable_info()->set_opcode(info.iOpcode);
	mux.mutable_info()->set_seq_num(info.iRPCRequestID);
	*(mux.mutable_role()->mutable_info()) = mux.info();
	mux.set_body_msg(msg);

	std::string channel = apie::event_ns::NatsManager::GetTopicChannel(server);

	::nats_msg::NATS_MSG_PRXOY nats_msg;
	(*nats_msg.mutable_multiplexer_forward()) = mux;
	return apie::event_ns::NatsSingleton::get().publishNatsMsg(apie::event_ns::NatsManager::E_NT_Realm, channel, nats_msg);
}

void ForwardManager::onForwardMuxMessage(const ::rpc_msg::RoleIdentifier& role, MessageInfo info, const std::string& msg)
{
	auto typeOpt = this->getType(info.iOpcode);
	if (!typeOpt.has_value())
	{
		std::stringstream ss;
		ss << "unregister|iOpcode:" << info.iOpcode;
		ASYNC_PIE_LOG(PIE_ERROR, "ForwardManager/onMessage|{}", ss.str().c_str());
		return;
	}

	std::string sType = typeOpt.value();
	auto ptrMsg = apie::message::ProtobufFactory::createMessage(sType);
	if (ptrMsg == nullptr)
	{
		std::stringstream ss;
		ss << "createMessage null|iOpcode:" << info.iOpcode << "|sType:" << sType;
		ASYNC_PIE_LOG(PIE_ERROR, "ForwardManager/onMessage|{}", ss.str().c_str());
		return;
	}

	std::shared_ptr<::google::protobuf::Message> newMsg(ptrMsg);
	bool bResult = newMsg->ParseFromString(msg);
	if (!bResult)
	{
		std::stringstream ss;
		ss << "ParseFromString error|iOpcode:" << info.iOpcode << "|sType:" << sType;
		ASYNC_PIE_LOG(PIE_ERROR, "ForwardManager/onMessage|{}", ss.str().c_str());
		return;
	}

	auto find_ite = func_.find(info.iOpcode);
	if (find_ite == func_.end())
	{
		//TODO
		return;
	}

	if (find_ite->second)
	{
		find_ite->second(role, newMsg);
	}
}

void ForwardManager::setDemuxCallback(DemuxCallback func)
{
	demux_callback_ = func;
}

void ForwardManager::onForwardDemuxMessage(const ::rpc_msg::RoleIdentifier& role, const std::string& msg)
{
	if (!demux_callback_)
	{
		return;
	}

	demux_callback_(role, msg);
}


MessageInfo ForwardManager::extractMessageInfo(const ::rpc_msg::RoleIdentifier& role)
{
	MessageInfo info;
	info.iSessionId = role.info().session_id();
	info.iRPCRequestID = role.info().seq_num();
	info.iOpcode = role.info().opcode();
	info.iResponseOpcode = role.info().response_opcode();

	switch (role.info().connetion_type())
	{
	case 1:
	{
		info.iConnetionType = ConnetionType::CT_CLIENT;
		break;
	}
	case 2:
	{
		info.iConnetionType = ConnetionType::CT_SERVER;
		break;
	}
	default:
		break;
	}

	return info;
}

bool ForwardManager::sendResponse(::rpc_msg::RoleIdentifier role, const ::google::protobuf::Message& msg)
{
	auto iResponseOpcode = role.info().response_opcode();

	::rpc_msg::PRC_DeMultiplexer_Forward demux;
	*demux.mutable_role() = role;
	*demux.mutable_info() = role.info();
	demux.mutable_info()->set_opcode(iResponseOpcode);
	*demux.mutable_role()->mutable_info() = demux.info();
	demux.set_body_msg(msg.SerializeAsString());

	std::string channel = apie::event_ns::NatsManager::GetTopicChannel(role.gw_id());

	::nats_msg::NATS_MSG_PRXOY nats_msg;
	(*nats_msg.mutable_demultiplexer_forward()) = demux;
	return apie::event_ns::NatsSingleton::get().publishNatsMsg(apie::event_ns::NatsManager::E_NT_Realm, channel, nats_msg);
}

bool ForwardManager::sendNotify(uint64_t iRoleId, ::rpc_msg::CHANNEL gwId, uint32_t iOpcode, const ::google::protobuf::Message& msg)
{
	::rpc_msg::PRC_DeMultiplexer_Forward demux;
	demux.mutable_role()->set_user_id(iRoleId);
	*demux.mutable_role()->mutable_gw_id() = gwId;
	demux.mutable_info()->set_opcode(iOpcode);
	*demux.mutable_role()->mutable_info() = demux.info();
	demux.set_body_msg(msg.SerializeAsString());

	std::string channel = apie::event_ns::NatsManager::GetTopicChannel(gwId);

	::nats_msg::NATS_MSG_PRXOY nats_msg;
	(*nats_msg.mutable_demultiplexer_forward()) = demux;
	return apie::event_ns::NatsSingleton::get().publishNatsMsg(apie::event_ns::NatsManager::E_NT_Realm, channel, nats_msg);
}

bool ForwardManager::sendNotifyToGW(uint64_t iRoleId, uint32_t iOpcode, const ::google::protobuf::Message& msg)
{
	::rpc_msg::PRC_DeMultiplexer_Forward demux;
	demux.mutable_role()->set_user_id(iRoleId);
	demux.mutable_info()->set_opcode(iOpcode);
	*demux.mutable_role()->mutable_info() = demux.info();
	demux.set_body_msg(msg.SerializeAsString());

	std::string channel = apie::event_ns::NatsManager::GenerateGWRIdChannel(iRoleId);

	::nats_msg::NATS_MSG_PRXOY nats_msg;
	(*nats_msg.mutable_demultiplexer_forward()) = demux;
	return apie::event_ns::NatsSingleton::get().publishNatsMsg(apie::event_ns::NatsManager::E_NT_Realm, channel, nats_msg, false);
}

}  
}
