#pragma once

#include <string>

#include "apie/proto/init.h"

namespace apie {
namespace forward {


class ForwardBase {
public:
	enum class RequestType
	{
		RT_Request = 0,
		RT_Notify,
	};

	using HandlerCb = std::function<void(::rpc_msg::RoleIdentifier, const std::shared_ptr<::google::protobuf::Message>&)>;

	explicit ForwardBase(uint32_t opcode, RequestType type)
		: opcode_(opcode),
		request_type_(type)
	{
	}

	virtual ~ForwardBase() {}

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

