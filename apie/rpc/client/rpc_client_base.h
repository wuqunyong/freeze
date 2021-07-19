#pragma once

#include <string>

#include "apie/proto/init.h"
#include "apie/status/status.h"

namespace apie {
namespace rpc {

class RPCClientBase : public std::enable_shared_from_this<RPCClientBase> {
public:
	explicit RPCClientBase(::rpc_msg::RPC_OPCODES opcode)
		: opcode_(opcode)
	{
	}

	virtual ~RPCClientBase() {}

	virtual void destroy() = 0;
	virtual void onMessage(const status::Status& status, const std::string& response_data) = 0;

	::rpc_msg::RPC_OPCODES opcode() const { return opcode_; }

protected:
	::rpc_msg::RPC_OPCODES opcode_;

};

}  
} 

