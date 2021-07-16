#pragma once

#include <string>

namespace apie {
namespace rpc {


class RPCServerBase {
public:


	explicit RPCServerBase(uint32_t opcode)
		: opcode_(opcode)
	{
	}

	virtual ~RPCServerBase() {}

	virtual void destroy() = 0;

	uint32_t opcode() const
	{
		return opcode_;
	}

protected:
	uint32_t opcode_;

};


} 
}

