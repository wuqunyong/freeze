#include "apie/rpc/client/rpc_client_contex.h"

namespace apie {
namespace rpc {

RPCClientContext::RPCClientContext(const ::rpc_msg::CHANNEL& server) :
	server_id_(server)
{

}

RPCClientContext::~RPCClientContext()
{

}
::rpc_msg::CHANNEL RPCClientContext::getServerId()
{
	return server_id_;
}

}
}

