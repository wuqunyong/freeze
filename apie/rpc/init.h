#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "../network/address.h"
#include "../network/windows_platform.h"

#include "../pb_msg.h"

#include <event2/util.h>

#include "apie/rpc/client/rpc_client_manager.h"

namespace apie {
namespace rpc {

	void rpcInit();

	template <typename Request, typename Response>
	bool RPC_Call(const ::rpc_msg::CHANNEL& server, ::rpc_msg::RPC_OPCODES opcode,  const Request& params, const typename RPCClient<Request, Response>::CallbackType& calllback)
	{
		auto rpcObj = RPCClientManagerSingleton::get().createRPCClient<Request, Response>(server, opcode, calllback);
		return rpcObj->sendRequest(params);
	}

}
}