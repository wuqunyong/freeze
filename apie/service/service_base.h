#pragma once

#include <string>
#include "apie/network/command.h"

namespace apie {
namespace service {


class ServiceBase {
public:
	using HandlerCb = std::function<void(MessageInfo, const std::shared_ptr<::google::protobuf::Message>&)>;

	enum class RequestType
	{
		RT_Request = 0,
		RT_Notify,
	};

	explicit ServiceBase(uint32_t opcode, RequestType type)
		: opcode_(opcode),
		request_type_(type)
	{
	}

	virtual ~ServiceBase() {}

	virtual void destroy() = 0;

	uint32_t opcode() const
	{
		return opcode_;
	}

	RequestType requestType() const
	{
		return request_type_;
	}

protected:
	uint32_t opcode_;
	RequestType request_type_;

};


} 
}

