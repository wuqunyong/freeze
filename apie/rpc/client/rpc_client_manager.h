#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <functional>
#include <optional>

#include <google/protobuf/message.h>

#include "apie/singleton/threadsafe_singleton.h"
#include "apie/rpc/client/rpc_client_base.h"

namespace apie {
namespace rpc {


class RPCClientManager {
public:
	RPCClientManager();
	~RPCClientManager();

	struct timer_info
	{
		uint64_t id_;
		uint64_t expire_at_;
	};

	uint64_t nextSeqNum();

	bool addPendingRequests(uint64_t seq_num, const std::shared_ptr<RPCClientBase>& ptr_request);
	void removePendingRequests(uint64_t seq_num);

	void handleTimeout();
	void handleResponse(uint64_t seq_num, const status::Status& status, const std::string& response_data);

	void insertRequestsTimeout(uint64_t seq_num, uint64_t expired_at);

private:

	uint64_t sequence_number_ = 0;
	std::unordered_map<uint64_t, std::shared_ptr<RPCClientBase>> pending_requests_;
	std::multimap<uint64_t, timer_info> expire_at_;
	uint64_t last_check_timeout_at_ = 0;
};


using RPCClientManagerSingleton = apie::ThreadSafeSingleton<RPCClientManager>;



}
}

