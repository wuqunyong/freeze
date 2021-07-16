#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <functional>
#include <optional>

#include <google/protobuf/message.h>

#include "apie/singleton/threadsafe_singleton.h"
#include "apie/rpc/client/rpc_client.h"
#include "apie/rpc/client/rpc_client_base.h"

namespace apie {
namespace rpc {


class RPCClientManager {
public:
	RPCClientManager();
	~RPCClientManager();


	template <typename Request, typename Response>
	auto createRPCClient(const std::string& server_id, uint32_t opcode, const typename RPCClient<Request, Response>::CallbackType& calllback)
		->std::shared_ptr<RPCClient<Request, Response>>;

	uint64_t nextSeqNum();
	bool addPendingRequests(uint64_t seq_num, const std::shared_ptr<RPCClientBase>& ptr_request);
	void removePendingRequests(uint64_t seq_num);

	void handleTimeout();
	void handleResponse(uint64_t seq_num, const status::Status& status, const std::string& response_data);

private:
	uint64_t sequence_number_ = 0;
	std::unordered_map<uint64_t, std::shared_ptr<RPCClientBase>> pending_requests_;
};

template <typename Request, typename Response>
auto RPCClientManager::createRPCClient(const std::string& server_id, uint32_t opcode, const typename RPCClient<Request, Response>::CallbackType& calllback)
	->std::shared_ptr<RPCClient<Request, Response>>
{
	auto client_ptr = std::make_shared<RPCClient<Request, Response>>(*this, server_id, opcode, calllback);
	return client_ptr;
}


using RPCClientManagerSingleton = APie::ThreadSafeSingleton<RPCClientManager>;



}
}

