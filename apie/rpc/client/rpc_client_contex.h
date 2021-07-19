#pragma once

#include <string>

#include "apie/pb_msg.h"

namespace apie {
namespace rpc {


class RPCClientContext {
public:
	enum class Type {
		UNARY,
		SERVER_STREAMING,
	};

	RPCClientContext(const ::rpc_msg::CHANNEL& server);
	~RPCClientContext();

	Type type() const { return type_; }
	::rpc_msg::CHANNEL getServerId();
	uint32_t timeoutMs();

private:
	::rpc_msg::CHANNEL client_id_;
	::rpc_msg::CHANNEL server_id_;
	Type type_ = Type::UNARY;
	uint32_t timeout_ms_ = 30000;
};


}
}

