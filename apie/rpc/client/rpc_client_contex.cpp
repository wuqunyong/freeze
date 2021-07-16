#include "apie/rpc/client/rpc_client_contex.h"

namespace apie {
namespace rpc {

RPCClientContext::RPCClientContext(const std::string& server_id, uint32_t opcode) :
	server_id_(server_id),
	opcode_(opcode)
{

}

RPCClientContext::~RPCClientContext()
{

}

}
}

