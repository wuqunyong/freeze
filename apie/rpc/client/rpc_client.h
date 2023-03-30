#pragma once

#include <functional>
#include <memory>

#include "apie/rpc/client/rpc_client_base.h"
#include "apie/rpc/client/rpc_client_contex.h"
#include "apie/rpc/client/rpc_client_manager.h"
#include "apie/status/status.h"
#include "apie/network/ctx.h"
#include "apie/event/nats_proxy.h"
#include "apie/common/protobuf_factory.h"
#include "apie/network/ctx.h"

namespace apie {
namespace rpc {

template <typename Request, typename Response>
class RPCClient : public RPCClientBase {
public:
	using SharedRequest = typename std::shared_ptr<Request>;
	using SharedResponse = typename std::shared_ptr<Response>;
	using CallbackType = std::function<void(const status::Status&, const SharedResponse&)>;

	template <typename RequestT, typename ResponseT>
	friend bool RPC_Call(const ::rpc_msg::CHANNEL& server, ::rpc_msg::RPC_OPCODES opcode, const RequestT& params, const typename RPCClient<RequestT, ResponseT>::CallbackType& calllback);

	template <typename RequestT, typename ResponseT>
	friend bool RPC_CallWithContext(const RPCClientContext& context, ::rpc_msg::RPC_OPCODES opcode, const RequestT& params, const typename RPCClient<RequestT, ResponseT>::CallbackType& calllback);

	RPCClient(const ::rpc_msg::CHANNEL& server, ::rpc_msg::RPC_OPCODES opcode, const CallbackType& callback)
		: RPCClientBase(opcode),
		manager_(RPCClientManagerSingleton::get()),
		callback_(callback),
		context_(server)
	{

	}

	RPCClient(const RPCClientContext& context, ::rpc_msg::RPC_OPCODES opcode, const CallbackType& callback)
		: RPCClientBase(opcode),
		manager_(RPCClientManagerSingleton::get()),
		callback_(callback),
		context_(context)
	{

	}


	RPCClient() = delete;

	virtual ~RPCClient() 
	{
		this->destroy();
	}

	void onMessage(const status::Status& status, const std::string& response_data) override;
	void destroy();

private:
	bool sendRequest(SharedRequest request);
	bool sendRequest(const Request& request);

	bool asyncSendRequest(SharedRequest request);
	void handleResponse(const status::Status& status, const SharedResponse& response);

	RPCClientManager& manager_;

	uint64_t sequence_number_;
	CallbackType callback_;

	RPCClientContext context_;
};

template <typename Request, typename Response>
void RPCClient<Request, Response>::destroy() {}


template <typename Request, typename Response>
bool RPCClient<Request, Response>::sendRequest(SharedRequest request)
{
	return asyncSendRequest(request);
}

template <typename Request, typename Response>
bool RPCClient<Request, Response>::sendRequest(const Request& request)
{
	auto request_ptr = std::make_shared<Request>(request);
	return sendRequest(request_ptr);
}


template <typename Request, typename Response>
bool RPCClient<Request, Response>::asyncSendRequest(SharedRequest request_ptr)
{
	auto seq_num = manager_.nextSeqNum();

	::rpc_msg::CHANNEL client = apie::Ctx::getThisChannel();

	::rpc_msg::RPC_REQUEST request;
	*request.mutable_client()->mutable_stub() = client;
	request.mutable_client()->set_seq_id(seq_num);
	request.mutable_client()->set_required_reply(false);

	*request.mutable_server()->mutable_stub() = context_.getServerId();
	request.set_opcodes(this->opcode());
	request.set_args_data(request_ptr->SerializeAsString());

	if (callback_ != nullptr)
	{
		request.mutable_client()->set_required_reply(true);

		bool result = manager_.addPendingRequests(seq_num, shared_from_this());
		if (!result)
		{
			return result;
		}
		uint64_t cur_time_ms = apie::CtxSingleton::get().getCurMilliseconds();
		auto expire_at = cur_time_ms + context_.getTimeoutMs();
		
		manager_.insertRequestsTimeout(seq_num, expire_at);
	}

	std::string channel = apie::event_ns::NatsManager::GetTopicChannel(request.server().stub());

	::nats_msg::NATS_MSG_PRXOY nats_msg;
	(*nats_msg.mutable_rpc_request()) = request;
	return apie::event_ns::NatsSingleton::get().publishNatsMsg(apie::event_ns::NatsManager::E_NT_Realm, channel, nats_msg);
}


template <typename Request, typename Response>
void RPCClient<Request, Response>::handleResponse(const status::Status& status, const SharedResponse& response)
{
	callback_(status, response);
}

template <typename Request, typename Response>
void RPCClient<Request, Response>::onMessage(const status::Status& status, const std::string& response_data)
{
	auto ptrMsg = apie::message::ProtobufFactory::createMessage(Response::descriptor()->full_name());
	if (ptrMsg == nullptr)
	{
		//TODO
		return;
	}

	std::shared_ptr<::google::protobuf::Message> newMsg(ptrMsg);
	bool bResult = newMsg->ParseFromString(response_data);
	if (!bResult)
	{
		//TODO
		return;
	}

	auto shared_obj = std::dynamic_pointer_cast<Response>(newMsg);
	if (shared_obj == nullptr)
	{
		//TODO
		return;
	}

	this->handleResponse(status, shared_obj);
}

}
}

