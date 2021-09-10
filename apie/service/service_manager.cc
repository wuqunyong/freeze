#include "apie/service/service_manager.h"

namespace apie {
namespace service {


ServiceManager::ServiceManager()
{
	this->init();
}

ServiceManager::~ServiceManager()
{
	this->destroy();
}

std::optional<std::string> ServiceManager::getType(uint32_t opcode)
{
	auto find_ite = type_.find(opcode);
	if (find_ite == type_.end())
	{
		return std::nullopt;
	}

	return find_ite->second;
}

void ServiceManager::init()
{

}

void ServiceManager::destroy()
{
	type_.clear();
	func_.clear();
	service_.clear();
}

ServiceManager::HandleMuxFunction& ServiceManager::getDefaultFunc()
{
	return default_func_;
}

void ServiceManager::setDefaultFunc(HandleMuxFunction func)
{
	default_func_ = func;
}

bool ServiceManager::sendResponse(MessageInfo info, const ::google::protobuf::Message& msg)
{
	MessageInfo response(info);
	response.iOpcode = info.iResponseOpcode;

	return apie::network::OutputStream::sendProtobufMsgImpl(info, msg);
}

bool ServiceManager::sendNotify(uint64_t iSessionId, uint32_t iOpcode, const ::google::protobuf::Message& msg)
{
	MessageInfo info;
	info.iSessionId = iSessionId;
	info.iOpcode = iOpcode;

	return apie::network::OutputStream::sendProtobufMsgImpl(info, msg);
}


}  
}
