#pragma once

#include <memory>
#include <string>
#include <functional>

#include "apie/proto/init.h"

#include "apie/status/status.h"
#include "apie/forward/forward_base.h"


namespace apie {
namespace forward {


template <typename Notify>
class HandleNotifyForward : public ForwardBase {
public:
	using ServiceCallback = std::function<void(const ::rpc_msg::RoleIdentifier&, const std::shared_ptr<Notify>&)>;

	HandleNotifyForward(uint32_t opcode, const ServiceCallback& service_callback)
		: ForwardBase(opcode, ForwardBase::RequestType::RT_Notify),
		service_callback_(service_callback)
	{
		this->init();
	}

	HandleNotifyForward() = delete;
	~HandleNotifyForward()
	{
		this->destroy();
	}

	void init();
	void destroy();

	std::function<void(::rpc_msg::RoleIdentifier, const std::shared_ptr<::google::protobuf::Message>&) > getHandler()
	{
		auto ptr_cb = [this](::rpc_msg::RoleIdentifier role, const std::shared_ptr<::google::protobuf::Message>& notify) {
			auto shared_obj = std::dynamic_pointer_cast<Notify>(notify);
			if (shared_obj == nullptr)
			{
				//TODO
				return;
			}

			this->handleNotify(role, shared_obj);
		};

		return ptr_cb;
	}

	void handleNotify(const ::rpc_msg::RoleIdentifier& role, const std::shared_ptr<Notify>& notify);

private:
	ServiceCallback service_callback_;
	std::function<void(uint64_t serial_num, const std::shared_ptr<Notify>&)> request_callback_;
};

template <typename Notify>
void HandleNotifyForward<Notify>::destroy()
{
}

template <typename Notify>
void HandleNotifyForward<Notify>::init()
{
	request_callback_ = std::bind(&HandleNotifyForward<Notify>::handleNotify, this, std::placeholders::_1, std::placeholders::_2);
}

template <typename Notify>
void HandleNotifyForward<Notify>::handleNotify(const ::rpc_msg::RoleIdentifier& role, const std::shared_ptr<Notify>& notify)
{
	service_callback_(role, notify);
}


} 
}
