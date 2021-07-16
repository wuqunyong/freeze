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

void RPCServerManager::onMessage(const ::rpc_msg::RPC_REQUEST& context)
{
	auto optType = this->getType(context.opcodes());
	if (!optType.has_value())
	{
		//TODO
		return;
	}

	auto ptrMsg = apie::message::ProtobufFactory::createMessage(optType.value());
	if (ptrMsg == nullptr)
	{
		//TODO
		return;
	}

	std::shared_ptr<::google::protobuf::Message> newMsg(ptrMsg);
	bool bResult = newMsg->ParseFromString(context.args_data());
	if (!bResult)
	{
		//TODO
		return;
	}

	auto find_ite = func_.find(context.opcodes());
	if (find_ite == func_.end())
	{
		//TODO
		return;
	}
	find_ite->second(context, newMsg);
}

}  
}
