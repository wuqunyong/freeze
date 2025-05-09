#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <functional>
#include <optional>
#include <sstream>
#include <mutex>

#include <google/protobuf/message.h>

#include "apie/proto/init.h"

#include "apie/singleton/threadsafe_singleton.h"
#include "apie/forward/handle_notify_forward.h"
#include "apie/forward/handle_request_forward.h"
#include "apie/forward/forward_base.h"
#include "apie/event/nats_proxy.h"
#include "apie/common/protobuf_factory.h"
#include "apie/network/command.h"

namespace apie {
namespace forward {

#define REGISTER_FORWARD_REQUEST(opcode, func)                                                                                                                                                              \
	{                                                                                                                                                                                                         \
		bool bResult = apie::forward::ForwardManagerSingleton::get().createService<opcode##Request, ::pb::core::OP_##opcode##Response, opcode##Response>(::pb::core::OP_##opcode##Request, func);             \
		if (!bResult) {                                                                                                                                                                                       \
			std::stringstream ss;                                                                                                                                                                             \
			ss << "forward register " << ::pb::core::OP_##opcode##Request << " collision";                                                                                                                    \
			PANIC_ABORT(ss.str().c_str());                                                                                                                                                                    \
		}                                                                                                                                                                                                     \
	}

#define REGISTER_FORWARD_NOTIFY(opcode, func)                                                                                                       \
	{                                                                                                                                                 \
		bool bResult = apie::forward::ForwardManagerSingleton::get().createService<opcode##Notify>(::pb::core::OP_##opcode##Notify, func);            \
		if (!bResult) {                                                                                                                               \
			std::stringstream ss;                                                                                                                     \
			ss << "forward register " << ::pb::core::OP_##opcode##Notify << " collision";                                                             \
			PANIC_ABORT(ss.str().c_str());                                                                                                            \
		}                                                                                                                                             \
	}


class ForwardManager {
public:
	using ServiceCallback = std::function<void(::rpc_msg::RoleIdentifier, const std::shared_ptr<::google::protobuf::Message>&)>;
	using DemuxCallback = std::function<void(const ::rpc_msg::RoleIdentifier& role, const std::string& msg)>;

	ForwardManager();
	virtual ~ForwardManager();

	void init();
	void destroy();

	std::optional<std::string> getType(uint32_t opcode);

	template <typename Request, uint32_t responseOpcode, typename Response>
	bool createService(uint32_t opcode, const typename HandleRequestForward<Request, responseOpcode, Response>::ServiceCallback& service_calllback);

	template <typename Notify>
	bool createService(uint32_t opcode, const typename HandleNotifyForward<Notify>::ServiceCallback& service_calllback);

	bool sendForwardMux(const ::rpc_msg::CHANNEL& server, const ::rpc_msg::RoleIdentifier& role, MessageInfo info, const std::string& msg);

	void onForwardMuxMessage_Head(const ::rpc_msg::RoleIdentifier& role, MessageInfo info, const std::string& msg);
	void onForwardMuxMessage_Tail(const ::rpc_msg::RoleIdentifier& role, MessageInfo info, std::shared_ptr<::google::protobuf::Message> ptrMsg);

	void setDemuxCallback(DemuxCallback func);
	void onForwardDemuxMessage(const ::rpc_msg::RoleIdentifier& role, const std::string& msg);

public:
	static MessageInfo extractMessageInfo(const ::rpc_msg::RoleIdentifier& role);

	static bool sendResponse(::rpc_msg::RoleIdentifier role, const ::google::protobuf::Message& msg);
	static bool sendNotify(uint64_t iRoleId, ::rpc_msg::CHANNEL gwId, uint32_t iOpcode, const ::google::protobuf::Message& msg);
	static bool sendNotifyToGW(uint64_t iRoleId, uint32_t iOpcode, const ::google::protobuf::Message& msg);

private:
	std::map<uint32_t, std::shared_ptr<ForwardBase>> service_;
	std::map<uint32_t, std::string> type_;
	std::map<uint32_t, ServiceCallback> func_;

	std::mutex type_sync_;

	DemuxCallback demux_callback_;
};


template <typename Request, uint32_t responseOpcode, typename Response>
bool ForwardManager::createService(
	uint32_t opcode, const typename HandleRequestForward<Request, responseOpcode, Response>::ServiceCallback& service_calllback)
{
	auto find_ite = service_.find(opcode);
	if (find_ite != service_.end())
	{
		std::stringstream ss;
		ss << "service_ register duplicate |iOpcode:" << opcode;
		ASYNC_PIE_LOG(PIE_ERROR, "Network|ForwardManager|createService|{}", ss.str());
		return false;
	}

	auto service_ptr = std::make_shared<HandleRequestForward<Request, responseOpcode, Response>>(opcode, service_calllback);
	service_[opcode] = service_ptr;

	std::string pb_type = Request::descriptor()->full_name();

	{
		std::lock_guard<std::mutex> guard(type_sync_);
		type_[opcode] = pb_type;
	}

	func_[opcode] = service_ptr->getHandler();

	return true;
}


template <typename Notify>
bool ForwardManager::createService(
	uint32_t opcode, const typename HandleNotifyForward<Notify>::ServiceCallback& service_calllback)
{
	auto find_ite = service_.find(opcode);
	if (find_ite != service_.end())
	{
		std::stringstream ss;
		ss << "service_ register duplicate |iOpcode:" << opcode;
		ASYNC_PIE_LOG(PIE_ERROR, "Network|ForwardManager|createService|{}", ss.str());
		return false;
	}

	auto service_ptr = std::make_shared<HandleNotifyForward<Notify>>(opcode, service_calllback);
	service_[opcode] = service_ptr;

	std::string pb_type = Notify::descriptor()->full_name();

	{
		std::lock_guard<std::mutex> guard(type_sync_);
		type_[opcode] = pb_type;
	}

	func_[opcode] = service_ptr->getHandler();

	return true;
}


using ForwardManagerSingleton = apie::ThreadSafeSingleton<ForwardManager>;


}  
}  

