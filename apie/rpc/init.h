#pragma once

#include <array>
#include <cstdint>
#include <string>

#include <event2/util.h>

#include "apie/network/address.h"
#include "apie/network/windows_platform.h"
#include "apie/proto/init.h"
#include "apie/rpc/client/rpc_client.h"

namespace apie {
namespace rpc {

	template <typename Request, typename Response>
	auto createRPCClient(const ::rpc_msg::CHANNEL& server, ::rpc_msg::RPC_OPCODES opcode, const typename RPCClient<Request, Response>::CallbackType& calllback)
		->std::shared_ptr<RPCClient<Request, Response>>
	{
		auto client_ptr = std::make_shared<RPCClient<Request, Response>>(RPCClientManagerSingleton::get(), server, opcode, calllback);
		return client_ptr;
	}

	template <typename Request, typename Response>
	auto createRPCClient(const RPCClientContext& context, ::rpc_msg::RPC_OPCODES opcode, const typename RPCClient<Request, Response>::CallbackType& calllback)
		->std::shared_ptr<RPCClient<Request, Response>>
	{
		auto client_ptr = std::make_shared<RPCClient<Request, Response>>(RPCClientManagerSingleton::get(), context, opcode, calllback);
		return client_ptr;
	}

	template <typename Request, typename Response>
	bool RPC_Call(const ::rpc_msg::CHANNEL& server, ::rpc_msg::RPC_OPCODES opcode,  const Request& params, const typename RPCClient<Request, Response>::CallbackType& calllback)
	{
		auto rpcObj = createRPCClient<Request, Response>(server, opcode, calllback);
		return rpcObj->sendRequest(params);
	}

	template <typename Request, typename Response>
	bool RPC_CallWithContext(const RPCClientContext& context, ::rpc_msg::RPC_OPCODES opcode, const Request& params, const typename RPCClient<Request, Response>::CallbackType& calllback)
	{
		auto rpcObj = createRPCClient<Request, Response>(context, opcode, calllback);
		return rpcObj->sendRequest(params);
	}



	void RPC_AsyncStreamReply(const rpc_msg::CLIENT_IDENTIFIER& client, uint32_t errCode, const std::string& replyData, bool hasMore, uint32_t offset);

}
}
