#pragma once

#include <memory>
#include <string>
#include <functional>

#include "apie/status/status.h"
#include "apie/service/service_base.h"
#include "apie/network/command.h"


namespace apie {
namespace service {


template <typename Notify>
class HandleNotifyService : public ServiceBase {
public:
	using ServiceCallback = std::function<void(MessageInfo, const std::shared_ptr<Notify>&)>;

	HandleNotifyService(uint32_t opcode, const ServiceCallback& service_callback)
		: ServiceBase(opcode, ServiceBase::RequestType::RT_Notify),
		service_callback_(service_callback)
	{
		this->init();
	}

	HandleNotifyService() = delete;
	~HandleNotifyService()
	{
		this->destroy();
	}

	void init();
	void destroy();

	std::function<void(MessageInfo, const std::shared_ptr<::google::protobuf::Message>&) > getHandler()
	{
		auto ptr_cb = [this](MessageInfo info, const std::shared_ptr<::google::protobuf::Message>& notify) {
			auto shared_obj = std::dynamic_pointer_cast<Notify>(notify);
			if (shared_obj == nullptr)
			{
				//TODO
				return;
			}

			this->handleNotify(info, shared_obj);
		};

		return ptr_cb;
	}

	void handleNotify(MessageInfo info, const std::shared_ptr<Notify>& notify);

private:
	ServiceCallback service_callback_;
	std::function<void(MessageInfo, const std::shared_ptr<Notify>&)> request_callback_;
};

template <typename Notify>
void HandleNotifyService<Notify>::destroy()
{
}

template <typename Notify>
void HandleNotifyService<Notify>::init()
{
	request_callback_ = std::bind(&HandleNotifyService<Notify>::handleNotify, this, std::placeholders::_1, std::placeholders::_2);
}

template <typename Notify>
void HandleNotifyService<Notify>::handleNotify(MessageInfo info, const std::shared_ptr<Notify>& notify)
{
	service_callback_(info, notify);
}


} 
}
