#include "apie/rpc/client/rpc_client_manager.h"

#include "apie/status/status.h"
#include "apie/network/ctx.h"
#include "apie/network/logger.h"

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
	uint64_t curTime = apie::Ctx::getCurMilliseconds();

	if (curTime > last_check_timeout_at_)
	{
		last_check_timeout_at_ = curTime + 1000;

		auto it = expire_at_.begin();
		while (it != expire_at_.end()) 
		{

			//  If we have to wait to execute the item, same will be true about
			//  all the following items (multimap is sorted). Thus we can stop
			//  checking the subsequent timers and return the time to wait for
			//  the next timer (at least 1ms).
			if (it->first > curTime)
				return;

			//  Trigger the timer.
			auto findIte = pending_requests_.find(it->second.id_);
			if (findIte != pending_requests_.end())
			{
				try {
					status::Status status(status::StatusCode::TIMEOUT, "timeout");
					findIte->second->onMessage(status, "");
				}
				catch (const std::exception& e) {
					std::stringstream ss;
					ss << "handleTimeout|exception:" << e.what();
					ASYNC_PIE_LOG(PIE_ERROR, "DispatcherImpl/exception|{}", ss.str().c_str());
				}

				pending_requests_.erase(findIte);
			}

			//  Remove it from the list of active timers.
			auto o = it;
			++it;
			expire_at_.erase(o);
		}
	}
}

void RPCClientManager::handleResponse(uint64_t seq_num, const status::Status& status, const std::string& response_data)
{
	auto find_ite = pending_requests_.find(seq_num);
	if (find_ite == pending_requests_.end())
	{
		//TODO
		return;
	}

	find_ite->second->onMessage(status, response_data);

	if (!status.hasMore())
	{
		pending_requests_.erase(find_ite);
	}

}

void RPCClientManager::insertRequestsTimeout(uint64_t seq_num, uint64_t expired_at)
{
	RPCClientManager::timer_info info = { seq_num, expired_at };
	expire_at_.insert(std::make_pair(expired_at, info));
}

}
}

