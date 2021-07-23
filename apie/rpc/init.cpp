#include "apie/rpc/init.h"

#include "apie/rpc/client/rpc_client.h"
#include "apie/rpc/server/rpc_server.h"

namespace apie {
namespace rpc {
	void RPC_AsyncStreamReply(const rpc_msg::CLIENT_IDENTIFIER& client, uint32_t errCode, const std::string& replyData, bool hasMore, uint32_t offset)
	{

		::rpc_msg::CHANNEL server = apie::Ctx::getThisChannel();

		::rpc_msg::RPC_RESPONSE response;
		*response.mutable_client() = client;
		*response.mutable_server()->mutable_stub() = server;
		response.mutable_status()->set_code(errCode);
		response.set_result_data(replyData);
		response.set_has_more(hasMore);
		response.set_offset(offset);

		std::string channel = apie::event_ns::NatsManager::GetTopicChannel(client.stub());

		::nats_msg::NATS_MSG_PRXOY nats_msg;
		(*nats_msg.mutable_rpc_response()) = response;
		apie::event_ns::NatsSingleton::get().publishNatsMsg(apie::event_ns::NatsManager::E_NT_Realm, channel, nats_msg);
	}
} 
} 
