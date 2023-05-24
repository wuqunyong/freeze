#pragma once

#include <string>

#include "apie/proto/init.h"
#include "apie/status/status.h"

namespace apie {
namespace rpc {

class RPCClientBase : public std::enable_shared_from_this<RPCClientBase> {
public:
	explicit RPCClientBase(uint32_t opcode)
		: opcode_(opcode)
	{
	}

	virtual ~RPCClientBase() {}

	virtual void destroy() = 0;
	virtual void onMessage(const status::Status& status, const std::string& response_data) = 0;

	uint32_t opcode() const { return opcode_; }

protected:
	uint32_t opcode_;

};

}  
} 

