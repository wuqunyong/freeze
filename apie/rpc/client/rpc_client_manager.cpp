#include "apie/rpc/client/rpc_client_manager.h"

namespace apie {
namespace rpc {

RPCClientManager::RPCClientManager()
{

}

RPCClientManager::~RPCClientManager()
{

}

uint64_t RPCClientManager::nextSeqNum()
{
	return ++sequence_number_; 
}

bool RPCClientManager::addPendingRequests(uint64_t seq_num, const std::shared_ptr<RPCClientBase>& ptr_request)
{
	auto find_ite = pending_requests_.find(seq_num);
	if (find_ite != pending_requests_.end())
	{
		//TODO
		return false;
	}

	pending_requests_[seq_num] = ptr_request;
	return true;
}


void RPCClientManager::removePendingRequests(uint64_t seq_num)
{
	pending_requests_.erase(seq_num);
}

void RPCClientManager::handleTimeout()
{

}

void RPCClientManager::handleResponse(uint64_t seq_num, const status::Status& status, const std::string& response_data)
{

}


}
}

