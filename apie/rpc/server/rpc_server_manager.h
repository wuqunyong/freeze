#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <functional>
#include <optional>

#include <google/protobuf/message.h>

#include "apie/singleton/threadsafe_singleton.h"
#include "apie/rpc/server/rpc_server.h"
#include "apie/rpc/server/rpc_server_base.h"
#include "apie/proto/init.h"
#include "apie/common/protobuf_factory.h"

namespace apie {
namespace rpc {


class RPCServerManager {
public:
	using ServiceCallback = std::function<void(const ::rpc_msg::RPC_REQUEST&, const std::shared_ptr<::google::protobuf::Message>&)>;

	RPCServerManager();
	virtual ~RPCServerManager();

	void init();
	void destroy();

	std::optional<std::string> getType(uint32_t opcode);

	template <typename Request, typename Response>
	bool createRPCServer(uint32_t opcode, const typename RPCServer<Request, Response>::ServiceCallback& service_calllback);


	void onMessage(const ::rpc_msg::RPC_REQUEST& request);


private:
	std::map<uint32_t, std::shared_ptr<RPCServerBase>> service_;
	std::map<uint32_t, std::string> type_;
	std::map<uint32_t, ServiceCallback> func_;
};


template <typename Request, typename Response>
bool RPCServerManager::createRPCServer(
	uint32_t opcode, const typename RPCServer<Request, Response>::ServiceCallback& service_calllback)
{
	auto find_ite = service_.find(opcode);
	if (find_ite != service_.end())
	{
		//TODO
		return false;
	}

	auto service_ptr = std::make_shared<RPCServer<Request, Response>>(opcode, service_calllback);
	service_[opcode] = service_ptr;

	std::string pb_type = Request::descriptor()->full_name();
	type_[opcode] = pb_type;
	func_[opcode] = service_ptr->getHandler();

	return true;
}

using RPCServerManagerSingleton = apie::ThreadSafeSingleton<RPCServerManager>;


}  
}  

