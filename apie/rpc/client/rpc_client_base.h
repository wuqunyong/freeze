#pragma once

#include <string>

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

	uint32_t opcode() const { return opcode_; }

protected:
	uint32_t opcode_;

};

}  
} 

