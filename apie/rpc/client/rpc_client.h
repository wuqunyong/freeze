#pragma once

#include <functional>
#include <memory>

#include "apie/rpc/client/rpc_client_base.h"
#include "apie/rpc/client/rpc_client_contex.h"

#include "apie/status/status.h"

namespace apie {
namespace rpc {

class RPCClientManager;

template <typename Request, typename Response>
class RPCClient : public RPCClientBase {
public:
	using SharedRequest = typename std::shared_ptr<Request>;
	using SharedResponse = typename std::shared_ptr<Response>;
	using CallbackType = std::function<void(const status::Status&, const SharedResponse&)>;

	RPCClient(RPCClientManager& manager, const std::string& server_id, uint32_t opcode, const CallbackType& callback)
		: RPCClientBase(opcode),
		manager_(manager),
		callback_(callback),
		context_(server_id, opcode)
	{
	}

	RPCClient() = delete;

	virtual ~RPCClient() 
	{
		this->destroy();
	}

	bool sendRequest(SharedRequest request);
	bool sendRequest(const Request& request);

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
	auto request_ptr = std::make_shared<const Request>(request);
	return sendRequest(request_ptr);
}


template <typename Request, typename Response>
bool RPCClient<Request, Response>::asyncSendRequest(SharedRequest request)
{
	auto seq_num = manager_.nextSeqNum();
	bool result = manager_.addPendingRequests(seq_num, shared_from_this());
	if (!result)
	{
		return result;
	}

	return true;
}


template <typename Request, typename Response>
void RPCClient<Request, Response>::handleResponse(const status::Status& status, const SharedResponse& response)
{
	callback_(status, response);
}


}
}

