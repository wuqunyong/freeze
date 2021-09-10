#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <functional>
#include <optional>
#include <sstream>

#include <google/protobuf/message.h>

#include "apie/singleton/threadsafe_singleton.h"

#include "apie/forward/handle_notify_forward.h"
#include "apie/forward/handle_request_forward.h"
#include "apie/forward/forward_base.h"

#include "apie/proto/init.h"
#include "apie/event/nats_proxy.h"
#include "apie/common/protobuf_factory.h"
#include "apie/network/command.h"

namespace apie {
namespace forward {

#define S_REGISTER_FORWARD_REQUEST(opcode, func)                                                                                                                                                              \
	{                                                                                                                                                                                                         \
		bool bResult = apie::forward::ForwardManagerSingleton::get().createService<MSG_REQUEST_##opcode, ::apie::OP_MSG_RESPONSE_##opcode, MSG_RESPONSE_##opcode>(::apie::OP_MSG_REQUEST_##opcode, func);     \
		if (!bResult) {                                                                                                                                                                                       \
			std::stringstream ss;                                                                                                                                                                             \
			ss << "forward register " << ::apie::OP_MSG_REQUEST_##opcode << " collision";                                                                                                                     \
			PANIC_ABORT(ss.str().c_str());                                                                                                                                                                    \
		}                                                                                                                                                                                                     \
	}

#define S_REGISTER_FORWARD_NOTIFY(opcode, func)                                                                                                       \
	{                                                                                                                                                 \
		bool bResult = apie::forward::ForwardManagerSingleton::get().createService<MSG_NOTIFY_##opcode>(::apie::OP_MSG_NOTIFY_##opcode, func);        \
		if (!bResult) {                                                                                                                               \
			std::stringstream ss;                                                                                                                     \
			ss << "forward register " << ::apie::OP_MSG_NOTIFY_##opcode << " collision";                                                              \
			PANIC_ABORT(ss.str().c_str());                                                                                                            \
		}                                                                                                                                             \
	}


class ForwardManager {
public:
	using ServiceCallback = std::function<void(const ::rpc_msg::RoleIdentifier&, const std::shared_ptr<::google::protobuf::Message>&)>;
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

	void onForwardMuxMessage(const ::rpc_msg::RoleIdentifier& role, MessageInfo info, const std::string& msg);

	void setDemuxCallback(DemuxCallback func);
	void onForwardDemuxMessage(const ::rpc_msg::RoleIdentifier& role, const std::string& msg);

public:
	static MessageInfo extractMessageInfo(const ::rpc_msg::RoleIdentifier& role);

	static bool sendResponse(::rpc_msg::RoleIdentifier role, const ::google::protobuf::Message& msg);
	static bool sendNotify(uint64_t iRoleId, ::rpc_msg::CHANNEL gwId, uint32_t iOpcode, const ::google::protobuf::Message& msg);

private:
	std::map<uint32_t, std::shared_ptr<ForwardBase>> service_;
	std::map<uint32_t, std::string> type_;
	std::map<uint32_t, ServiceCallback> func_;

	DemuxCallback demux_callback_;
};


template <typename Request, uint32_t responseOpcode, typename Response>
bool ForwardManager::createService(
	uint32_t opcode, const typename HandleRequestForward<Request, responseOpcode, Response>::ServiceCallback& service_calllback)
{
	auto find_ite = service_.find(opcode);
	if (find_ite != service_.end())
	{
		//TODO

		return false;
	}

	auto service_ptr = std::make_shared<HandleRequestForward<Request, responseOpcode, Response>>(opcode, service_calllback);
	service_[opcode] = service_ptr;

	std::string pb_type = Request::descriptor()->full_name();
	type_[opcode] = pb_type;
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
		//TODO
		return false;
	}

	auto service_ptr = std::make_shared<HandleNotifyForward<Notify>>(opcode, service_calllback);
	service_[opcode] = service_ptr;

	std::string pb_type = Notify::descriptor()->full_name();
	type_[opcode] = pb_type;
	func_[opcode] = service_ptr->getHandler();

	return true;
}


using ForwardManagerSingleton = apie::ThreadSafeSingleton<ForwardManager>;


}  
}  

