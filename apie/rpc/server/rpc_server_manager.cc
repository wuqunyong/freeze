#include "apie/rpc/server/rpc_server_manager.h"

namespace apie {
namespace rpc {


RPCServerManager::RPCServerManager()
{
	this->init();
}

RPCServerManager::~RPCServerManager()
{
	this->destroy();
}

std::optional<std::string> RPCServerManager::getType(uint32_t opcode)
{
	std::lock_guard<std::mutex> guard(type_sync_);

	auto find_ite = type_.find(opcode);
	if (find_ite == type_.end())
	{
		return std::nullopt;
	}

	return find_ite->second;
}

void RPCServerManager::init()
{

}

void RPCServerManager::destroy()
{
	type_.clear();
	func_.clear();
	service_.clear();
}

void RPCServerManager::sendResponseError(const apie::status::Status& status, const ::rpc_msg::RPC_REQUEST& context)
{
	std::string channel = apie::event_ns::NatsManager::GetTopicChannel(context.client().stub());

	::rpc_msg::CHANNEL server = apie::Ctx::getThisChannel();

	::rpc_msg::RPC_RESPONSE response;
	*response.mutable_client() = context.client();
	*response.mutable_server()->mutable_stub() = server;

	response.mutable_status()->set_code(apie::toUnderlyingType(status.code()));
	response.mutable_status()->set_msg(status.message());

	response.set_result_data("");

	::nats_msg::NATS_MSG_PRXOY nats_msg;
	(*nats_msg.mutable_rpc_response()) = response;
	apie::event_ns::NatsSingleton::get().publishNatsMsg(apie::event_ns::NatsManager::E_NT_Realm, channel, nats_msg);
}

void RPCServerManager::onMessage_Head(const ::rpc_msg::RPC_REQUEST& context)
{
	auto optType = this->getType(context.opcodes());
	if (!optType.has_value())
	{
		std::stringstream ss;
		ss << "getType opcodes:" << context.opcodes();

		apie::status::Status status(apie::status::StatusCode::RPC_Request_UnRegister, ss.str());
		sendResponseError(status, context);
		return;
	}

	auto ptrMsg = apie::message::ProtobufFactory::createMessage(optType.value());
	if (ptrMsg == nullptr)
	{
		std::stringstream ss;
		ss << "createMessage:" << optType.value();

		apie::status::Status status(apie::status::StatusCode::RPC_Request_createMessage, ss.str());
		sendResponseError(status, context);
		return;
	}

	std::shared_ptr<::google::protobuf::Message> newMsg(ptrMsg);
	bool bResult = newMsg->ParseFromString(context.args_data());
	if (!bResult)
	{
		ASYNC_PIE_LOG(PIE_ERROR, "RPCServerManager | onMessage | ParseFromString | opcode:{} | type: {} | data:{}", context.opcodes(), optType.value(), context.args_data());
		
		std::stringstream ss;
		ss << "ParseFromString Error";

		apie::status::Status status(apie::status::StatusCode::RPC_Request_ParseFromString, ss.str());
		sendResponseError(status, context);
		return;
	}

	::rpc_msg::RPC_REQUEST rpcContext;
	(*rpcContext.mutable_client()) = context.client();
	(*rpcContext.mutable_server()) = context.server();
	rpcContext.set_server_stream(context.server_stream());
	rpcContext.set_opcodes(context.opcodes());
	apie::CtxSingleton::get().getLogicThread()->dispatcher().post(
		[rpcContext, newMsg, this]() mutable {
			onMessage_Tail(rpcContext, newMsg);
		}
	);
}

void RPCServerManager::onMessage_Tail(const ::rpc_msg::RPC_REQUEST& context, std::shared_ptr<::google::protobuf::Message> ptrNewMsg)
{
	auto find_ite = func_.find(context.opcodes());
	if (find_ite == func_.end())
	{
		std::stringstream ss;
		ss << "func_ opcodes:" << context.opcodes();

		apie::status::Status status(apie::status::StatusCode::RPC_Request_UnRegister, ss.str());
		sendResponseError(status, context);
		return;
	}
	find_ite->second(context, ptrNewMsg);
}

}  
}
