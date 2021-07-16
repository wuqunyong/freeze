#pragma once

#include <string>

namespace apie {
namespace rpc {


class RPCClientContext {
public:
	enum class Type {
		UNARY,
		SERVER_STREAMING,
	};

	RPCClientContext(const std::string& server_id, uint32_t opcode);
	~RPCClientContext();

	Type type() const { return type_; }


private:
	std::string client_id_;
	std::string server_id_;
	uint32_t opcode_;
	Type type_;
};


}
}

