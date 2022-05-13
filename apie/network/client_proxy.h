#pragma once

#include <string>
#include <stdint.h>
#include <queue>
#include <memory>
#include <queue>
#include <functional>
#include <map>
#include <future>

#include <google/protobuf/message.h>

#include "apie/network/i_poll_events.hpp"
#include "apie/network/command.h"
#include "apie/event/timer.h"
#include "apie/network/output_stream.h"

#include "apie/proto/init.h"

namespace apie
{
	class ClientProxy : public std::enable_shared_from_this<ClientProxy>
	{
	public:
		//参数：iResult:0(连接成功)
		//返回值：true:出错后进程尝试，false:出错后调用close
		using HandleConnectCB = std::function<bool(apie::ClientProxy* ptrClient, uint32_t iResult)>;

		using HeartbeatCB = std::function<void(ClientProxy* ptrClient)>;

		enum CONNECT_STATE
		{
			CONNECT_CLOSE = 0,
			CONNECT_ESTABLISHED,
		};

		ClientProxy();
		~ClientProxy();
		bool checkTag();

		uint32_t getTId();

		int connect(const std::string& ip, uint16_t port, ProtocolType type, uint32_t maskFlag, HandleConnectCB cb=nullptr);
		void resetConnect(const std::string& ip, uint16_t port, ProtocolType type);
		int reconnect();

		bool syncConnect(const std::string& ip, uint16_t port, ProtocolType type, uint32_t maskFlag, HandleConnectCB cb = nullptr);

		void addReconnectTimer(uint64_t interval);
		void disableReconnectTimer();

		void setHeartbeatCb(HeartbeatCB cb);
		void addHeartbeatTimer(uint64_t interval);
		void disableHeartbeatTimer();

		uint32_t getReconnectTimes();
		void onConnect(uint32_t iResult);
		void onPassiveClose(uint32_t iResult, const std::string& sInfo, uint32_t iActiveClose);
		void onActiveClose();

		void onRecvPackage(uint64_t iSerialNum, ::google::protobuf::Message* ptrMsg);

		int32_t sendMsg(uint32_t iOpcode, const ::google::protobuf::Message& msg);
		uint64_t getSequenceNumber();

		template <typename Response>
		std::shared_ptr<Response> syncSendMsg(uint32_t iOpcode, const ::google::protobuf::Message& msg);

		uint64_t getSerialNum();
		std::string getCurSerialNumToStr();

		ProtocolType getCodecType();
		std::string getHosts();

		bool isConnectted();
		void setHadEstablished(uint32_t value);

		event_ns::TimerPtr& reconnectTimer();

		void setLocalIp(const std::string& ip);

		int64_t getUserId();
		void setUserId(int64_t iUserId);

	private:
		void close();
		int sendConnect();
		void sendClose();
		
		std::shared_future<std::shared_ptr<service_discovery::ConnectDialResult>> syncSendConnect();

		static uint64_t generatorId();

	public:
		static bool registerClientProxy(std::shared_ptr<ClientProxy> ptrClient);
		static void unregisterClientProxy(uint64_t iSerialNum);
		static std::shared_ptr<ClientProxy> findClientProxy(uint64_t iSerialNum);

		static void clearAllClientProxy();

		static std::shared_ptr<ClientProxy> createClientProxy();

	private:
		uint32_t m_tag;
		uint64_t m_curSerialNum;
		std::string m_localIp;

		uint64_t m_sequenceNumber = 0;

		std::string m_ip;
		uint16_t m_port;
		ProtocolType m_codecType;  //协议：1(PB√),2(HTTP)
		uint32_t m_maskFlag = 0;   //按位表示：0b1u(压缩√,客户端<->Gateway),0b10u(加密×)
		HandleConnectCB m_cb;

		uint32_t m_hadEstablished; //当前的连接状态：0：未连接，1：已连上
		uint32_t m_reconnectTimes;

		event_ns::TimerPtr m_reconnectTimer;

		event_ns::TimerPtr m_heartbeatTimer;
		HeartbeatCB m_heartbeatCb;

		uint32_t m_tId;            //附着IO线程ID

		int64_t m_iUserId = 0;

		static std::mutex m_sync;
		static std::map<uint64_t, std::shared_ptr<ClientProxy>> m_clientProxy;
	};

	template <typename Response>
	std::shared_ptr<Response> ClientProxy::syncSendMsg(uint32_t iOpcode, const ::google::protobuf::Message& msg)
	{
		if (this->m_hadEstablished != CONNECT_ESTABLISHED)
		{
			return nullptr;
		}

		m_sequenceNumber++;

		MessageInfo info;
		info.iSessionId = this->m_curSerialNum;
		info.iSeqNum = m_sequenceNumber;
		info.iOpcode = iOpcode;
		info.iConnetionType = apie::ConnetionType::CT_CLIENT;
		auto future = apie::network::syncSendProtobufMsgImpl<Response>(info, msg);

		if (!future.valid())
		{
			return nullptr;
		}

		std::chrono::seconds timeout_s(10);
		auto status = future.wait_for(timeout_s);
		if (status == std::future_status::ready) {
			return future.get();
		}
		else {
			return nullptr;
		}
	}
}
