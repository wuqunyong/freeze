#pragma once

#include <functional>
#include <memory>

#include "apie/rpc/client/rpc_client_base.h"
#include "apie/rpc/client/rpc_client_contex.h"

#include "apie/status/status.h"
#include "apie/network/ctx.h"
#include "apie/event/nats_proxy.h"
#include "apie/common/protobuf_factory.h"

namespace apie {
namespace rpc {

class RPCClientManager;

template <typename Request, typename Response>
class RPCClient : public RPCClientBase {
public:
	using SharedRequest = typename std::shared_ptr<Request>;
	using SharedResponse = typename std::shared_ptr<Response>;
	using CallbackType = std::function<void(const status::Status&, const SharedResponse&)>;

	RPCClient(RPCClientManager& manager, const ::rpc_msg::CHANNEL& server, ::rpc_msg::RPC_OPCODES opcode, const CallbackType& callback)
		: RPCClientBase(opcode),
		manager_(manager),
		callback_(callback),
		context_(server)
	{
	}

	RPCClient() = delete;

	virtual ~RPCClient() 
	{
		this->destroy();
	}

	bool sendRequest(SharedRequest request);
	bool sendRequest(const Request& request);

	void onMessage(const status::Status& status, const std::string& response_data) override;

	void destroy();


private:
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

	::rpc_msg::CHANNEL client;
	client.set_type(APie::CtxSingleton::get().identify().type);
	client.set_id(APie::CtxSingleton::get().identify().id);
	client.set_realm(APie::CtxSingleton::get().identify().realm);


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
	}

	bool bResult = false;
	std::string channel = APie::Event::NatsManager::GetTopicChannel(request.server().stub().realm(), request.server().stub().type(), request.server().stub().id());

	::nats_msg::NATS_MSG_PRXOY nats_msg;
	(*nats_msg.mutable_rpc_request()) = request;
	int32_t iRC = APie::Event::NatsSingleton::get().publishNatsMsg(APie::Event::NatsManager::E_NT_Realm, channel, nats_msg);
	if (iRC == 0)
	{
		bResult = true;
	}

	return true;
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

