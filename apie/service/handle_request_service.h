#pragma once

#include <memory>
#include <string>
#include <functional>

#include "apie/status/status.h"
#include "apie/network/output_stream.h"
#include "apie/network/command.h"

namespace apie {
namespace service {


template <typename Request, uint32_t responseOpcode, typename Response>
class HandleRequestService : public ServiceBase {
public:
	using ServiceCallback = std::function<status::E_ReturnType(MessageInfo, const std::shared_ptr<Request>&, std::shared_ptr<Response>&)>;

	HandleRequestService(uint32_t opcode, const ServiceCallback& service_callback)
		: ServiceBase(opcode, ServiceBase::RequestType::RT_Request),
		service_callback_(service_callback)
	{
		this->init();
	}

	HandleRequestService() = delete;
	~HandleRequestService()
	{
		this->destroy();
	}

	void init();
	void destroy();

	HandlerCb getHandler()
	{
		auto ptr_cb = [this](MessageInfo info, const std::shared_ptr<::google::protobuf::Message>& request)
		{
			auto shared_obj = std::dynamic_pointer_cast<Request>(request);
			if (shared_obj == nullptr)
			{
				//TODO
				return;
			}

			this->handleRequest(info, shared_obj);
		};

		return ptr_cb;
	}

	void handleRequest(MessageInfo info, const std::shared_ptr<Request>& request);

private:
	void sendResponse(MessageInfo info, const std::shared_ptr<Response>& response);

	ServiceCallback service_callback_;
	std::function<void(MessageInfo, const std::shared_ptr<Request>&)> request_callback_;

	const uint32_t responseOpcode_ = responseOpcode;
};

template <typename Request, uint32_t responseOpcode, typename Response>
void HandleRequestService<Request, responseOpcode, Response>::destroy()
{
}

template <typename Request, uint32_t responseOpcode, typename Response>
void HandleRequestService<Request, responseOpcode, Response>::init()
{
	request_callback_ = std::bind(&HandleRequestService<Request, responseOpcode, Response>::handleRequest, this, std::placeholders::_1, std::placeholders::_2);
}

template <typename Request, uint32_t responseOpcode, typename Response>
void HandleRequestService<Request, responseOpcode, Response>::handleRequest(MessageInfo info, const std::shared_ptr<Request>& request)
{
	info.iResponseOpcode = responseOpcode_;

	auto response = std::make_shared<Response>();

	auto status = service_callback_(info, request, response);
	if (status == apie::status::E_ReturnType::kRT_Sync)
	{
		sendResponse(info, response);
	}
}

template <typename Request, uint32_t responseOpcode, typename Response>
void HandleRequestService<Request, responseOpcode, Response>::sendResponse(MessageInfo info, const std::shared_ptr<Response>& response)
{
	info.iOpcode = info.iResponseOpcode;
	apie::network::OutputStream::sendProtobufMsgImpl(info, *response);
}


} 
}
