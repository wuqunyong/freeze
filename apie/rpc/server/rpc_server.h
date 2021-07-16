#pragma once

#include <memory>
#include <string>
#include <functional>

#include "apie/rpc/server/rpc_server_base.h"
#include "apie/status/status.h"
#include "apie/pb_msg.h"
#include "apie/network/ctx.h"
#include "apie/event/nats_proxy.h"

namespace apie {
namespace rpc {


template <typename Request, typename Response>
class RPCServer : public RPCServerBase {
public:
	using ServiceCallback = std::function<status::Status(const ::rpc_msg::CLIENT_IDENTIFIER&, const std::shared_ptr<Request>&, std::shared_ptr<Response>&)>;

	RPCServer(uint32_t opcode, const ServiceCallback& service_callback)
		: RPCServerBase(opcode),
		service_callback_(service_callback)
	{
		this->init();
	}

	RPCServer() = delete;
	~RPCServer()
	{
		this->destroy();
	}

	void init();
	void destroy();

	std::function<void(const ::rpc_msg::RPC_REQUEST&, const std::shared_ptr<::google::protobuf::Message>&) > getHandler()
	{
		auto ptr_cb = [this](const ::rpc_msg::RPC_REQUEST& context, const std::shared_ptr<::google::protobuf::Message>& request)
		{
			auto shared_obj = std::dynamic_pointer_cast<Request>(request);
			if (shared_obj == nullptr)
			{
				//TODO
				return;
			}

			this->handleRequest(context, shared_obj);
		};

		return ptr_cb;
	}

	void handleRequest(const ::rpc_msg::RPC_REQUEST& context, const std::shared_ptr<Request>& request);

private:
	void sendResponse(const ::rpc_msg::RPC_REQUEST& context, const std::shared_ptr<Response>& response);

	ServiceCallback service_callback_;
	std::function<void(const ::rpc_msg::RPC_REQUEST&, const std::shared_ptr<Request>&)> request_callback_;
};

template <typename Request, typename Response>
void RPCServer<Request, Response>::destroy()
{
}

template <typename Request, typename Response>
void RPCServer<Request, Response>::init()
{
	request_callback_ = std::bind(&RPCServer<Request, Response>::handleRequest, this, std::placeholders::_1, std::placeholders::_2);
}

template <typename Request, typename Response>
void RPCServer<Request, Response>::handleRequest(const ::rpc_msg::RPC_REQUEST& context, const std::shared_ptr<Request>& request)
{
	auto response = std::make_shared<Response>();
	auto status = service_callback_(context.client(), request, response);
	if (status.ok())
	{
		if (!status.isAsync())
		{
			if (!context.client().required_reply())
			{
				return;
			}

			sendResponse(context, response);
		}
	}
}

template <typename Request, typename Response>
void RPCServer<Request, Response>::sendResponse(const ::rpc_msg::RPC_REQUEST& context, const std::shared_ptr<Response>& response_ptr)
{
	std::cout << context.ShortDebugString() << response_ptr->ShortDebugString() << std::endl;

	std::string channel = APie::Event::NatsManager::GetTopicChannel(context.client().stub().realm(), context.client().stub().type(), context.client().stub().id());

	::rpc_msg::CHANNEL server;
	server.set_realm(APie::CtxSingleton::get().identify().realm);
	server.set_type(APie::CtxSingleton::get().identify().type);
	server.set_id(APie::CtxSingleton::get().identify().id);

	::rpc_msg::RPC_RESPONSE response;
	*response.mutable_client() = context.client();
	*response.mutable_server()->mutable_stub() = server;


	response.mutable_status()->set_code(1);
	response.set_result_data(response_ptr->SerializeAsString());

	::nats_msg::NATS_MSG_PRXOY nats_msg;
	(*nats_msg.mutable_rpc_response()) = response;
	APie::Event::NatsSingleton::get().publishNatsMsg(APie::Event::NatsManager::E_NT_Realm, channel, nats_msg);
}


} 
}
