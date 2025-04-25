#pragma once

#include <array>
#include <cstdint>
#include <string>

#include <event2/util.h>

#include "apie/proto/init.h"

#include "apie/network/address.h"
#include "apie/network/windows_platform.h"
#include "apie/rpc/client/rpc_client.h"

namespace apie {
namespace rpc {

	template <typename Request, typename Response>
	auto createRPCClient(const ::rpc_msg::CHANNEL& server, uint32_t opcode, const typename RPCClient<Request, Response>::CallbackType& calllback)
		->std::shared_ptr<RPCClient<Request, Response>>
	{
		auto client_ptr = std::make_shared<RPCClient<Request, Response>>(server, opcode, calllback);
		return client_ptr;
	}

	template <typename Request, typename Response>
	auto createRPCClient(const RPCClientContext& context, uint32_t opcode, const typename RPCClient<Request, Response>::CallbackType& calllback)
		->std::shared_ptr<RPCClient<Request, Response>>
	{
		auto client_ptr = std::make_shared<RPCClient<Request, Response>>(context, opcode, calllback);
		return client_ptr;
	}

	template <typename Request, typename Response>
	bool RPC_Call(const ::rpc_msg::CHANNEL& server, uint32_t opcode,  const Request& params, const typename RPCClient<Request, Response>::CallbackType& calllback)
	{
		auto rpcObj = createRPCClient<Request, Response>(server, opcode, calllback);
		return rpcObj->sendRequest(params);
	}

	template <typename Request, typename Response>
	bool RPC_CallWithContext(const RPCClientContext& context, uint32_t opcode, const Request& params, const typename RPCClient<Request, Response>::CallbackType& calllback)
	{
		auto rpcObj = createRPCClient<Request, Response>(context, opcode, calllback);
		return rpcObj->sendRequest(params);
	}



	template <typename Notify>
	bool RPC_CallNotify(const ::rpc_msg::CHANNEL& server, uint32_t opcode, const Notify& notify)
	{
		auto seq_num = RPCClientManagerSingleton::get().nextSeqNum();

		::rpc_msg::CHANNEL client = apie::Ctx::getThisChannel();

		::rpc_msg::RPC_REQUEST request;
		*request.mutable_client()->mutable_stub() = client;
		request.mutable_client()->set_seq_id(seq_num);
		request.mutable_client()->set_required_reply(false);

		RPCClientContext context(server);
		*request.mutable_server()->mutable_stub() = context.getServerId();
		request.set_opcodes(opcode);
		request.set_args_data(notify.SerializeAsString());


		std::string channel = apie::event_ns::NatsManager::GetTopicChannel(request.server().stub());

		::nats_msg::NATS_MSG_PRXOY nats_msg;
		(*nats_msg.mutable_rpc_request()) = request;
		return apie::event_ns::NatsSingleton::get().publishNatsMsg(apie::event_ns::NatsManager::E_NT_Realm, channel, nats_msg);
	}


	void RPC_AsyncStreamReply(const rpc_msg::CLIENT_IDENTIFIER& client, uint32_t errCode, const std::string& replyData, bool hasMore, uint32_t offset);


	template <typename Response,
		typename std::enable_if<std::is_base_of<google::protobuf::Message, Response>::value,
		bool>::type = 0>
	void RPC_AsyncSendResponse(const apie::status::Status& status, const rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<Response>& response_ptr)
	{
		if (!client.required_reply())
		{
			return;
		}

		std::string channel = apie::event_ns::NatsManager::GetTopicChannel(client.stub());

		::rpc_msg::CHANNEL server = apie::Ctx::getThisChannel();

		::rpc_msg::RPC_RESPONSE response;
		*response.mutable_client() = client;
		*response.mutable_server()->mutable_stub() = server;

		response.mutable_status()->set_code(apie::toUnderlyingType(status.code()));
		response.mutable_status()->set_msg(status.message());

		response.set_result_data(response_ptr->SerializeAsString());

		::nats_msg::NATS_MSG_PRXOY nats_msg;
		(*nats_msg.mutable_rpc_response()) = response;
		apie::event_ns::NatsSingleton::get().publishNatsMsg(apie::event_ns::NatsManager::E_NT_Realm, channel, nats_msg);
	}

}
}
