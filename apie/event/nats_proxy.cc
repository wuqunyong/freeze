#include "apie/event/nats_proxy.h"

#include "third_party/influxdb-cpp/influxdb.hpp"
#include "nats/adapters/libevent.h"

#include "apie/rpc/server/rpc_server.h"
#include "apie/rpc/client/rpc_client.h"
#include "apie/rpc/client/rpc_client_manager.h"
#include "apie/rpc/server/rpc_server_manager.h"
#include "apie/status/status.h"
#include "apie/forward/forward_manager.h"

namespace apie {
namespace event_ns {

int32_t NATSConnectorBase::ConnectBase(struct event_base* ptrBase) 
{
	natsOptions* nats_opts = nullptr;
	natsOptions_Create(&nats_opts);

	if (ptrBase == nullptr)
	{
		ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|connect|event_base null");
		return 1;
	}

	natsLibevent_Init();

	if (tls_config_ != nullptr)
	{
		natsOptions_SetSecure(nats_opts, true);
		natsOptions_LoadCATrustedCertificates(nats_opts, tls_config_->ca_cert.c_str());
		natsOptions_LoadCertificatesChain(nats_opts, tls_config_->tls_cert.c_str(), tls_config_->tls_key.c_str());
	}

	auto s = natsOptions_SetEventLoop(nats_opts, ptrBase, natsLibevent_Attach, natsLibevent_Read, natsLibevent_Write, natsLibevent_Detach);
	if (s != NATS_OK)
	{
		std::stringstream ss;
		ss << "Failed to set NATS event loop, nats_status=" << s;
		ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|connect|{}", ss.str());

		return 2;
	}

	natsOptions_SetURL(nats_opts, nats_server_.c_str());

	//natsOptions_SetTimeout(nats_opts, config.connectionTimeout.count());
	//natsOptions_SetMaxReconnect(nats_opts, config.maxReconnectionAttempts);
	//natsOptions_SetReconnectWait(nats_opts, config.reconnectWait);

	natsOptions_SetClosedCB(nats_opts, ClosedCb, this);
	natsOptions_SetDisconnectedCB(nats_opts, DisconnectedCb, this);
	natsOptions_SetReconnectedCB(nats_opts, ReconnectedCb, this);
	natsOptions_SetMaxPendingMsgs(nats_opts, 65536);
	natsOptions_SetErrorHandler(nats_opts, ErrHandler, this);

	auto nats_status = natsConnection_Connect(&nats_connection_, nats_opts);
	natsOptions_Destroy(nats_opts);
	nats_opts = nullptr;

	if (nats_status != NATS_OK)
	{
		std::stringstream ss;
		ss << "Failed to connect to NATS, nats_status=" << s;
		ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|connet|{}", ss.str());
		return 3;
	}

	ASYNC_PIE_LOG(PIE_NOTICE, "nats/proxy|connect success|{}", nats_server_);

	return 0;
}

void NATSConnectorBase::DisconnectedCb(natsConnection* nc, void* closure)
{
	int32_t status = -1;
	if (nc != nullptr)
	{
		status = natsConnection_Status(nc);
	}

	std::thread::id iThreadId = std::this_thread::get_id();

	std::stringstream ss;
	ss << "DisconnectedCb|status:" << status << "|threadId:" << iThreadId;
	ASYNC_PIE_LOG(PIE_WARNING, "nats/proxy|status|{}", ss.str().c_str());
}

void NATSConnectorBase::ReconnectedCb(natsConnection* nc, void* closure)
{
	int32_t status = -1;
	if (nc != nullptr)
	{
		status = natsConnection_Status(nc);
	}

	if (status == NATS_CONN_STATUS_CONNECTED)
	{
		auto* connector = static_cast<NATSConnectorBase*>(closure);
		connector->conn_closed = false;
	}

	std::thread::id iThreadId = std::this_thread::get_id();

	std::stringstream ss;
	ss << "ReconnectedCb|status:" << status << "|threadId:" << iThreadId;
	ASYNC_PIE_LOG(PIE_WARNING, "nats/proxy|status|%s", ss.str().c_str());
}

void NATSConnectorBase::ClosedCb(natsConnection* nc, void* closure)
{
	auto* connector = static_cast<NATSConnectorBase*>(closure);
	connector->conn_closed = true;

	int32_t status = -1;
	if (nc != nullptr)
	{
		status = natsConnection_Status(nc);
	}

	std::thread::id iThreadId = std::this_thread::get_id();

	std::stringstream ss;
	ss << "ClosedCb|status:" << status << "|threadId:" << iThreadId;
	ASYNC_PIE_LOG(PIE_NOTICE, "nats/proxy|status|{}", ss.str().c_str());
}

void NATSConnectorBase::ErrHandler(natsConnection* nc, natsSubscription* subscription, natsStatus err, void* closure)
{
	int32_t status = -1;
	if (nc != nullptr)
	{
		status = natsConnection_Status(nc);
	}

	std::stringstream ss;
	if (err == NATS_SLOW_CONSUMER)
	{
		ss << "ErrHandler|status:" << status;
		ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|nats runtime error: slow consumer|{}", ss.str());
	}
	else
	{
		ss << "ErrHandler|status:" << status;
		ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|nats runtime error|{}", ss.str());
	}
}

NatsManager::NatsManager() : nats_realm(nullptr)
{

}

NatsManager::~NatsManager()
{
	if (interval_timer_)
	{
		interval_timer_->disableTimer();
		interval_timer_.reset(nullptr);
	}
}

bool NatsManager::init()
{
	auto bEnable = apie::CtxSingleton::get().getConfigs()->nats.enable;
	if (!bEnable)
	{
		return true;
	}

	uint32_t realm = apie::CtxSingleton::get().getServerRealm();
	uint32_t id = apie::CtxSingleton::get().getServerId();
	uint32_t type = apie::CtxSingleton::get().getServerType();

	for (const auto& elems : apie::CtxSingleton::get().getConfigs()->nats.connections)
	{
		auto iType = elems.type;
		auto sServer = elems.nats_server;
		auto sDomains = elems.channel_domains;

		switch (iType)
		{
		case E_NT_Realm:
		{
			nats_realm = createConnection(realm, type, id, iType, sServer, sDomains);
			if (nats_realm == nullptr)
			{
				return false;
			}
			break;
		}
		default:
		{
			return false;
		}
		}
	}

	interval_timer_ = apie::CtxSingleton::get().getNatsThread()->dispatcherImpl()->createTimer([this]() -> void { runIntervalCallbacks(); });
	interval_timer_->enableTimer(std::chrono::milliseconds(2000));

	return true;
}

void NatsManager::addConnection(uint32_t domainsType, const std::string& urls, const std::string& domains)
{
	uint32_t realm = apie::CtxSingleton::get().getServerRealm();
	uint32_t id = apie::CtxSingleton::get().getServerId();
	uint32_t type = apie::CtxSingleton::get().getServerType();

	switch (domainsType)
	{
	case E_NT_Realm:
	{
		if (nats_realm)
		{
			std::stringstream ss;
			ss << "duplicate type:" << domainsType;
			PANIC_ABORT(ss.str().c_str());
		}

		nats_realm = createConnection(realm, type, id, domainsType, urls, domains);
		if (nats_realm == nullptr)
		{
			std::stringstream ss;
			ss << "createConnection null type:" << domainsType;
			PANIC_ABORT(ss.str().c_str());
		}
		break;
	}
	default:
	{
		//nothing
	}
	}

	if (interval_timer_ == nullptr)
	{
		interval_timer_ = apie::CtxSingleton::get().getNatsThread()->dispatcherImpl()->createTimer([this]() -> void { runIntervalCallbacks(); });
		interval_timer_->enableTimer(std::chrono::milliseconds(2000));
	}
}

std::shared_ptr<NatsManager::PrxoyNATSConnector> NatsManager::createConnection(uint32_t realm, uint32_t serverType, uint32_t serverId, uint32_t domainsType, const std::string& urls, const std::string& domains)
{
	auto sharedPtr = std::make_shared<PrxoyNATSConnector>(urls, domains, domains);
	if (sharedPtr == nullptr)
	{
		return nullptr;
	}

	std::string channel = apie::event_ns::NatsManager::GetTopicChannel(realm, serverType, serverId);
	struct event_base* ptrBase = &(apie::CtxSingleton::get().getNatsThread()->dispatcherImpl()->base());
	int32_t iRC = sharedPtr->Connect(ptrBase, channel);
	if (iRC != 0)
	{
		return nullptr;
	}


	// Attach the message handler for agent nats:
	sharedPtr->RegisterMessageHandler(std::bind(&NatsManager::NATSMessageHandler, this, domainsType, std::placeholders::_1));
	return sharedPtr;
}

void NatsManager::destroy()
{
	if (interval_timer_)
	{
		interval_timer_->disableTimer();
		interval_timer_.reset(nullptr);
	}

	if (nats_realm)
	{
		nats_realm->destroy();
		nats_realm = nullptr;
	}
}

bool NatsManager::isConnect(E_NatsType type)
{
	switch (type)
	{
	case apie::event_ns::NatsManager::E_NT_Realm:
	{
		if (nats_realm == nullptr)
		{
			return false;
		}

		return nats_realm->isConnect();
	}
	default:
	{
		return false;
	}
	}

	return false;
}

bool NatsManager::publishNatsMsg(E_NatsType type, const std::string& channel, const PrxoyNATSConnector::OriginType& msg, bool bSplice)
{
	switch (type)
	{
	case apie::event_ns::NatsManager::E_NT_Realm:
	{
		if (nats_realm == nullptr)
		{
			std::stringstream ss;
			ss << "nats_realm nullptr";
			ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|publish|channel:{}|{}", channel.c_str(), ss.str().c_str());

			return false;
		}

		return nats_realm->Publish(channel, msg, bSplice);
	}
	default:
		break;
	}

	std::stringstream ss;
	ss << "invalid type:" << type;
	ASYNC_PIE_LOG(PIE_ERROR, "publish|channel:%s|%s", channel.c_str(), ss.str().c_str());

	return false;
}

bool NatsManager::subscribeChannel(E_NatsType type, const std::string& channel)
{
	switch (type)
	{
	case apie::event_ns::NatsManager::E_NT_Realm:
	{
		if (nats_realm == nullptr)
		{
			std::stringstream ss;
			ss << "nats_realm nullptr";
			ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|publish|channel:{}|{}", channel.c_str(), ss.str().c_str());

			return false;
		}

		return nats_realm->subscribeChannel(channel);
	}
	default:
		break;
	}

	std::stringstream ss;
	ss << "invalid type:" << type;
	ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|subscribeChannel|channel:{}|{}", channel.c_str(), ss.str().c_str());

	return false;
}

bool NatsManager::unsubscribeChannel(E_NatsType type, const std::string& channel)
{
	switch (type)
	{
	case apie::event_ns::NatsManager::E_NT_Realm:
	{
		if (nats_realm == nullptr)
		{
			std::stringstream ss;
			ss << "nats_realm nullptr";
			ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|publish|channel:{}|{}", channel.c_str(), ss.str().c_str());

			return false;
		}

		return nats_realm->unsubscribeChannel(channel);
	}
	default:
		break;
	}

	std::stringstream ss;
	ss << "invalid type:" << type;
	ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|unsubscribeChannel|channel:{}|{}", channel.c_str(), ss.str().c_str());

	return false;
}


void NatsManager::NATSMessageHandler(uint32_t type, PrxoyNATSConnector::MsgType msg)
{
	if (apie::CtxSingleton::get().getLogicThread() == nullptr)
	{
		std::thread::id iThreadId = std::this_thread::get_id();

		std::stringstream ss;
		ss << "msgHandle|ThreadId:" << iThreadId << "type:" << type << "|" << msg->ShortDebugString().c_str();
		ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy:{}", ss.str());

		return;
	}

	{
		std::lock_guard<std::mutex> guard(_sync);

		if (msg->has_rpc_request())
		{
			::rpc_msg::RPC_REQUEST request = msg->rpc_request();
			std::string sMetricsChannel = NatsManager::GetMetricsChannel(request.client().stub(), request.server().stub());
			channel_request_msgs[sMetricsChannel] = channel_request_msgs[sMetricsChannel] + 1;
		}

		if (msg->has_rpc_response())
		{
			::rpc_msg::RPC_RESPONSE response = msg->rpc_response();

			std::string sMetricsChannel = NatsManager::GetMetricsChannel(response.client().stub(), response.server().stub());
			channel_response_msgs[sMetricsChannel] = channel_response_msgs[sMetricsChannel] + 1;
		}
	}

	switch (type)
	{
	case apie::event_ns::NatsManager::E_NT_Realm:
	{
		//::nats_msg::NATS_MSG_PRXOY* m = msg.release();
		//apie::CtxSingleton::get().getLogicThread()->dispatcher().post(
		//	[m, this]() mutable { Handle_RealmSubscribe(std::unique_ptr<::nats_msg::NATS_MSG_PRXOY>(m)); }
		//);

		
		Handle_RealmSubscribe(msg);
		break;
	}
	default:
	{
		std::thread::id iThreadId = std::this_thread::get_id();

		std::stringstream ss;
		ss << "msgHandle|ThreadId:" << iThreadId << "type:" << type << "|" << msg->ShortDebugString().c_str();
		ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|{}", ss.str());
	}
	}

}

void NatsManager::runIntervalCallbacks()
{
	//std::thread::id iThreadId = std::this_thread::get_id();

	bool enable = apie::CtxSingleton::get().getConfigs()->metrics.enable;
	if (enable)
	{
		MetricData* ptrData = new MetricData;
		ptrData->sMetric = "queue";

		auto iType = apie::CtxSingleton::get().getServerType();
		auto iId = apie::CtxSingleton::get().getServerId();

		ptrData->tag["server_type"] = std::to_string(iType);
		ptrData->tag["server_id"] = std::to_string(iId);
		ptrData->tag["queue_id"] = std::to_string(iType) + "_" + std::to_string(iId) + "_nats";

		std::lock_guard<std::mutex> guard(_sync);

		uint32_t iRequestMsgs = 0;
		uint32_t iResponseMsgs = 0;
		for (auto& elems : channel_request_msgs)
		{
			iRequestMsgs += elems.second;

			std::string sChannel = "request_" + elems.first;
			ptrData->field[sChannel] = (double)elems.second;
		}

		for (auto& elems : channel_response_msgs)
		{
			iResponseMsgs += elems.second;

			std::string sChannel = "response_" + elems.first;
			ptrData->field[sChannel] = (double)elems.second;
		}
		ptrData->field["iRequestMsgs"] = (double)iRequestMsgs;
		ptrData->field["iResponseMsgs"] = (double)iResponseMsgs;

		channel_request_msgs.clear();
		channel_response_msgs.clear();

		Command command;
		command.type = Command::metric_data;
		command.args.metric_data.ptrData = ptrData;

		auto ptrMetric = apie::CtxSingleton::get().getMetricsThread();
		if (ptrMetric != nullptr)
		{
			ptrMetric->push(command);
		}
	}

	interval_timer_->enableTimer(std::chrono::milliseconds(1000));
}

void NatsManager::Handle_RealmSubscribe(std::unique_ptr<::nats_msg::NATS_MSG_PRXOY>& msg)
{
	//ASYNC_PIE_LOG(PIE_DEBUG, "nats/proxy|Handle_Subscribe|{}|{}", ss.str(), msg->ShortDebugString());

	if (msg->has_rpc_request())
	{
		apie::rpc::RPCServerManagerSingleton::get().onMessage_Head(msg->rpc_request());
		return;
	}

	if (msg->has_rpc_response())
	{
		::rpc_msg::RPC_RESPONSE response = msg->rpc_response();

		auto code = static_cast<apie::status::StatusCode>(response.status().code());

		apie::status::Status status(code, response.status().msg());
		status.setHasMore(response.has_more());

		apie::CtxSingleton::get().getLogicThread()->dispatcher().post(
			[response, status]() mutable {
				apie::rpc::RPCClientManagerSingleton::get().handleResponse(response.client().seq_id(), status, response.result_data());
			}
		);
		return;
	}

	if (msg->has_multiplexer_forward())
	{
		::rpc_msg::RoleIdentifier role = msg->multiplexer_forward().role();

		MessageInfo info;
		info.iRPCRequestID = msg->multiplexer_forward().info().seq_num();
		info.iOpcode = msg->multiplexer_forward().info().opcode();

		std::string sBodyMsg = msg->multiplexer_forward().body_msg();

		apie::CtxSingleton::get().getLogicThread()->dispatcher().post(
			[role, info, sBodyMsg]() mutable {
				apie::forward::ForwardManagerSingleton::get().onForwardMuxMessage(role, info, sBodyMsg);
			}
		);
		return;
	}

	if (msg->has_demultiplexer_forward())
	{
		::rpc_msg::RoleIdentifier role = msg->demultiplexer_forward().role();
		std::string sBodyMsg = msg->demultiplexer_forward().body_msg();

		apie::CtxSingleton::get().getLogicThread()->dispatcher().post(
			[role, sBodyMsg]() mutable {
				apie::forward::ForwardManagerSingleton::get().onForwardDemuxMessage(role, sBodyMsg);
			}
		);
		return;
	}

	std::thread::id iThreadId = std::this_thread::get_id();

	std::stringstream ss;
	ss << "ThreadId:" << iThreadId;
	ASYNC_PIE_LOG(PIE_ERROR, "nats/proxy|Handle_Subscribe invalid params|{}|{}", ss.str(), msg->ShortDebugString());
}

std::string NATSConnectorBase::GetCombineTopicChannel(const std::string& domains, const std::string& channel)
{
	std::string topic = domains + "." + channel;
	return topic;
}

std::string NatsManager::GetTopicChannel(uint32_t realm, uint32_t type, uint32_t id)
{
	std::string channel = std::to_string(realm) + "." + std::to_string(type) + "." + std::to_string(id);
	return channel;
}

std::string NatsManager::GetTopicChannel(const ::rpc_msg::CHANNEL& channel)
{
	return GetTopicChannel(channel.realm(), channel.type(), channel.id());
}

std::string NatsManager::GetMetricsChannel(const ::rpc_msg::CHANNEL& src, const ::rpc_msg::CHANNEL& dest)
{
	std::string channel = std::to_string(src.realm()) + "." + std::to_string(src.type()) + "." + std::to_string(src.id()) 
		+ "->" + std::to_string(dest.realm()) + "." + std::to_string(dest.type()) + "." + std::to_string(dest.id());
	return channel;
}

std::string NatsManager::GenerateGWRIdChannel(uint64_t iRId)
{
	std::stringstream ss;
	ss << "gateway.role." << iRId;
	return ss.str();
}

bool NatsManager::SubscribeChannelByRIdFromGW(uint64_t iRId)
{
	auto channel = GenerateGWRIdChannel(iRId);
	return apie::event_ns::NatsSingleton::get().subscribeChannel(apie::event_ns::NatsManager::E_NT_Realm, channel);
}

bool NatsManager::UnsubscribeChannelByRIdFromGW(uint64_t iRId)
{
	auto channel = GenerateGWRIdChannel(iRId);
	return apie::event_ns::NatsSingleton::get().unsubscribeChannel(apie::event_ns::NatsManager::E_NT_Realm, channel);
}

}  // namespace APie
}  // namespace Event
