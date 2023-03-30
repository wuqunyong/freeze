#pragma once

#include <string>

#include "apie/proto/init.h"

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

	Type getType() const { return type_; }
	void setType(Type type);

	::rpc_msg::CHANNEL getServerId();
	uint32_t getTimeoutMs();

private:
	::rpc_msg::CHANNEL client_id_;
	::rpc_msg::CHANNEL server_id_;
	Type type_ = Type::UNARY;
	uint32_t timeout_ms_ = 60000;
};


}
}

