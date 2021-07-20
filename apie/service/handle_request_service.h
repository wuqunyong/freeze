#pragma once

#include <memory>
#include <string>
#include <functional>

#include "apie/status/status.h"
#include "apie/network/output_stream.h"

namespace apie {
namespace service {


template <typename Request, uint32_t responseOpcode, typename Response>
class HandleRequestService : public ServiceBase {
public:
	using ServiceCallback = std::function<status::Status(uint64_t, const std::shared_ptr<Request>&, std::shared_ptr<Response>&)>;

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

	std::function<void(uint64_t, const std::shared_ptr<::google::protobuf::Message>&) > getHandler()
	{
		auto ptr_cb = [this](uint64_t serial_num, const std::shared_ptr<::google::protobuf::Message>& request)
		{
			auto shared_obj = std::dynamic_pointer_cast<Request>(request);
			if (shared_obj == nullptr)
			{
				//TODO
				return;
			}

			this->handleRequest(serial_num, shared_obj);
		};

		return ptr_cb;
	}

	void handleRequest(uint64_t serial_num, const std::shared_ptr<Request>& request);

private:
	void sendResponse(uint64_t serial_num, const std::shared_ptr<Response>& response);

	ServiceCallback service_callback_;
	std::function<void(uint64_t serial_num, const std::shared_ptr<Request>&)> request_callback_;

	uint32_t responseOpcode_ = responseOpcode;
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
void HandleRequestService<Request, responseOpcode, Response>::handleRequest(uint64_t serial_num, const std::shared_ptr<Request>& request)
{
	auto response = std::make_shared<Response>();
	auto status = service_callback_(serial_num, request, response);
	if (status.isAsync())
	{
		return;
	}

	if (status.ok())
	{
		sendResponse(serial_num, response);
	}
}

template <typename Request, uint32_t responseOpcode, typename Response>
void HandleRequestService<Request, responseOpcode, Response>::sendResponse(uint64_t serial_num, const std::shared_ptr<Response>& response)
{
	apie::network::OutputStream::sendMsg(serial_num, responseOpcode, *response);
}


} 
}
