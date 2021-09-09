#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <functional>
#include <optional>

#include <google/protobuf/message.h>

#include "apie/singleton/threadsafe_singleton.h"
#include "apie/service/handle_notify_service.h"
#include "apie/service/handle_request_service.h"
#include "apie/service/service_base.h"

#include "apie/network/command.h"

namespace apie {
namespace service {

#define S_REGISTER_SERVICE(opcode, func)                                                                                                                                                                     \
  {                                                                                                                                                                                                          \
  	bool bResult = apie::service::ServiceHandlerSingleton::get().server.createService<MSG_REQUEST_##opcode, ::apie::OP_MSG_RESPONSE_##opcode, MSG_RESPONSE_##opcode>(::apie::OP_MSG_REQUEST_##opcode, func); \
  	if (!bResult) {                                                                                                                                                                                          \
		std::stringstream ss;                                                                                                                                                                                \
		ss << "server register " << ::apie::OP_MSG_REQUEST_##opcode << " collision";                                                                                                                         \
		PANIC_ABORT(ss.str().c_str());                                                                                                                                                                       \
	}                                                                                                                                                                                                        \
  }


#define S_INTRA_REGISTER_SERVICE(opcode, func)                                                                                                                                                                    \
  {                                                                                                                                                                                                               \
  	bool bResult = apie::service::ServiceHandlerSingleton::get().server.createService<MSG_REQUEST_##opcode, ::opcodes::OP_MSG_RESPONSE_##opcode, MSG_RESPONSE_##opcode>(::opcodes::OP_MSG_REQUEST_##opcode, func);\
  	if (!bResult) {                                                                                                                                                                                               \
		std::stringstream ss;                                                                                                                                                                                     \
		ss << "server register " << ::opcodes::OP_MSG_REQUEST_##opcode << " collision";                                                                                                                           \
		PANIC_ABORT(ss.str().c_str());                                                                                                                                                                            \
	}                                                                                                                                                                                                             \
  }

class ServiceManager {
public:
	using ServiceCallback = std::function<void(MessageInfo, const std::shared_ptr<::google::protobuf::Message>&)>;
	using HandleMuxFunction = std::function<void(MessageInfo info, const std::string& msg)>;

	ServiceManager();
	virtual ~ServiceManager();

	void init();
	void destroy();

	std::optional<std::string> getType(uint32_t opcode);

	template <typename Request, uint32_t responseOpcode, typename Response>
	bool createService(uint32_t opcode, const typename HandleRequestService<Request, responseOpcode, Response>::ServiceCallback& service_calllback);

	template <typename Notify>
	bool createService(uint32_t opcode, const typename HandleNotifyService<Notify>::ServiceCallback& service_calllback);

	template <typename T>
	void onMessage(MessageInfo info, const std::shared_ptr<T>& message);

	HandleMuxFunction& getDefaultFunc();
	void setDefaultFunc(HandleMuxFunction func);

private:
	std::map<uint32_t, std::shared_ptr<ServiceBase>> service_;
	std::map<uint32_t, std::string> type_;
	std::map<uint32_t, ServiceCallback> func_;

	HandleMuxFunction default_func_;
};


template <typename Request, uint32_t responseOpcode, typename Response>
bool ServiceManager::createService(
	uint32_t opcode, const typename HandleRequestService<Request, responseOpcode, Response>::ServiceCallback& service_calllback)
{
	auto find_ite = service_.find(opcode);
	if (find_ite != service_.end())
	{
		//TODO

		return false;
	}

	auto service_ptr = std::make_shared<HandleRequestService<Request, responseOpcode, Response>>(opcode, service_calllback);
	service_[opcode] = service_ptr;

	std::string pb_type = Request::descriptor()->full_name();
	type_[opcode] = pb_type;
	func_[opcode] = service_ptr->getHandler();

	return true;
}


template <typename Notify>
bool ServiceManager::createService(
	uint32_t opcode, const typename HandleNotifyService<Notify>::ServiceCallback& service_calllback)
{
	auto find_ite = service_.find(opcode);
	if (find_ite != service_.end())
	{
		//TODO
		return false;
	}

	auto service_ptr = std::make_shared<HandleNotifyService<Notify>>(opcode, service_calllback);
	service_[opcode] = service_ptr;

	std::string pb_type = Notify::descriptor()->full_name();
	type_[opcode] = pb_type;
	func_[opcode] = service_ptr->getHandler();

	return true;
}

template <typename T>
void ServiceManager::onMessage(MessageInfo info, const std::shared_ptr<T>& message)
{
	auto find_ite = func_.find(info.iOpcode);
	if (find_ite == func_.end())
	{
		//TODO
		return;
	}

	find_ite->second(info, message);
}

struct ServiceHandler
{
	ServiceManager client;
	ServiceManager server;
};


using ServiceHandlerSingleton = apie::ThreadSafeSingleton<ServiceHandler>;


}  
}  

