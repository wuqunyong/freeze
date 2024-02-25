#pragma once

#include <memory>
#include <string>
#include <functional>
#include <iostream>
#include <thread>
#include <future>
#include <exception>

#include <google/protobuf/message.h>

#include "apie/network/logger.h"

namespace apie {
namespace service {

class SyncServiceBase : public std::enable_shared_from_this<SyncServiceBase> {
public:
	using ServiceCallback = std::function<void(const std::shared_ptr<::google::protobuf::Message>&)>;
	
	virtual ServiceCallback getHandler() = 0;
	virtual void setException(const std::exception& e) = 0;
};


template <typename Response>
class SyncService : public SyncServiceBase {
public:
	using ServiceCallback = std::function<void(const std::shared_ptr<::google::protobuf::Message>&)>;
	
	using SharedResponse = typename std::shared_ptr<Response>;
	using Promise = std::promise<SharedResponse>;
	using SharedFuture = std::shared_future<SharedResponse>;

	SyncService(std::string info) :
		info_(info)
	{

	}

	ServiceCallback getHandler() override
	{
		auto sInfo = info_;
		std::weak_ptr<SyncService<Response>> weak_this = std::dynamic_pointer_cast<SyncService<Response>>(shared_from_this());
		auto ptr_cb = [weak_this, sInfo](const std::shared_ptr<::google::protobuf::Message>& response)
		{
			auto share_this = weak_this.lock();
			if (!share_this) 
			{
				PIE_LOG(PIE_NOTICE, "Network|SyncService|{}", sInfo);
				return;
			}

			auto shared_obj = std::dynamic_pointer_cast<Response>(response);
			if (shared_obj == nullptr)
			{
				share_this->setException(std::invalid_argument("dynamic_pointer_cast error"));
				return;
			}

			share_this->promise_.set_value(shared_obj);
		};

		return ptr_cb;
	}

	void setException(const std::exception& exceptionObj) override
	{
		try
		{
			throw exceptionObj;
		}
		catch (const std::exception& e)
		{
			std::exception_ptr p = std::current_exception();
			this->promise_.set_exception(p);
		}
	}

	SharedFuture getFuture()
	{
		return promise_.get_future().share();
	}

private:
	Promise promise_;
	std::string info_;
};



} 
}
