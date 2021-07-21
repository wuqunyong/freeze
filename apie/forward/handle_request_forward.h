#pragma once

#include <memory>
#include <string>
#include <functional>

#include "apie/proto/init.h"
#include "apie/status/status.h"
#include "apie/network/output_stream.h"

namespace apie {
namespace forward {


template <typename Request, uint32_t responseOpcode, typename Response>
class HandleRequestForward : public ForwardBase {
public:
	using ServiceCallback = std::function<status::Status(const ::rpc_msg::RoleIdentifier&, const std::shared_ptr<Request>&, std::shared_ptr<Response>&)>;

	HandleRequestForward(uint32_t opcode, const ServiceCallback& service_callback)
		: ForwardBase(opcode, ForwardBase::RequestType::RT_Request),
		service_callback_(service_callback)
	{
		this->init();
	}

	HandleRequestForward() = delete;
	~HandleRequestForward()
	{
		this->destroy();
	}

	void init();
	void destroy();

	std::function<void(const ::rpc_msg::RoleIdentifier&, const std::shared_ptr<::google::protobuf::Message>&) > getHandler()
	{
		auto ptr_cb = [this](const ::rpc_msg::RoleIdentifier& role, const std::shared_ptr<::google::protobuf::Message>& request)
		{
			auto shared_obj = std::dynamic_pointer_cast<Request>(request);
			if (shared_obj == nullptr)
			{
				//TODO
				return;
			}

			this->handleRequest(role, shared_obj);
		};

		return ptr_cb;
	}

	void handleRequest(const ::rpc_msg::RoleIdentifier& role, const std::shared_ptr<Request>& request);

private:
	void sendResponse(const ::rpc_msg::RoleIdentifier& role, const std::shared_ptr<Response>& response);

	ServiceCallback service_callback_;
	std::function<void(const ::rpc_msg::RoleIdentifier&, const std::shared_ptr<Request>&)> request_callback_;

	uint32_t responseOpcode_ = responseOpcode;
};

template <typename Request, uint32_t responseOpcode, typename Response>
void HandleRequestForward<Request, responseOpcode, Response>::destroy()
{
}

template <typename Request, uint32_t responseOpcode, typename Response>
void HandleRequestForward<Request, responseOpcode, Response>::init()
{
	request_callback_ = std::bind(&HandleRequestForward<Request, responseOpcode, Response>::handleRequest, this, std::placeholders::_1, std::placeholders::_2);
}

template <typename Request, uint32_t responseOpcode, typename Response>
void HandleRequestForward<Request, responseOpcode, Response>::handleRequest(const ::rpc_msg::RoleIdentifier& role, const std::shared_ptr<Request>& request)
{
	auto response = std::make_shared<Response>();
	auto status = service_callback_(role, request, response);
	if (status.isAsync())
	{
		return;
	}

	if (status.ok())
	{
		sendResponse(role, response);
	}
}

template <typename Request, uint32_t responseOpcode, typename Response>
void HandleRequestForward<Request, responseOpcode, Response>::sendResponse(const ::rpc_msg::RoleIdentifier& role, const std::shared_ptr<Response>& response)
{
}


} 
}
