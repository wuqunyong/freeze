#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <future>
#include <map>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <utility>

#include <event2/bufferevent.h>

#include "apie/network/windows_platform.h"
#include "apie/network/i_poll_events.hpp"
#include "apie/network/command.h"

#include "apie/http/http_response_decoder.h"


namespace apie
{
	struct SetClientSessionAttr;

	enum E_DialResult
	{
		E_DR_SUCCESS = 0,
		E_DR_FAILURE_SocketNew = 1,
		E_DR_FAILURE_SocketConnect = 2,
		E_DR_FAILURE_GetFd = 3,
		E_DR_FAILURE_New = 4,
	};

    class ClientConnection :
        public i_poll_events
    {
    public:
		typedef uint64_t integer_t;
		ClientConnection(integer_t iSerialNum, bufferevent *bev, std::string address, short port, ProtocolType type, uint32_t threadId);

		uint64_t getSerialNum();
		uint32_t getTId();

		void readcb();
		void writecb();
		void eventcb(short what);

		void SetConnectTo(const std::string& sAddress, uint16_t iPort);

		void handleSetClientSessionAttr(SetClientSessionAttr* ptrCmd);
		std::optional<std::string> getSessionKey();

		void close(std::string sInfo, int iCode=0, int iActive=0);
        ~ClientConnection();

	public:
		void handleSend(const char *data, size_t size);
		void handleClose();

		void resetDialSync(std::shared_ptr<apie::service::SyncServiceBase> ptrDialSyncBase);
		bool addSyncSend(uint64_t iId, std::shared_ptr<apie::service::SyncServiceBase> ptrSync);
		
		bool triggerSyncHandler(uint64_t iId, const std::shared_ptr<::google::protobuf::Message>& response);


	private:
		void readHttp();
		void readPB();
		void readPBMsgHead();
		void readPBMsgUser();

		void recv(MessageInfo info, std::string& requestStr);


		void sendCloseCmd(uint32_t iResult, const std::string& sInfo, uint32_t iActive);
		void sendConnectResultCmd(uint32_t iResult);

	public:
		static std::shared_ptr<ClientConnection> createClient(uint32_t threadId, struct event_base *base, DialParameters* ptrDial);

    private:
		integer_t iSerialNum;
		bufferevent *bev;
		ProtocolType iType;
		std::optional<std::string>  m_optSessionKey;

		std::string sListenAddress;
		uint16_t iListenPort;

		std::string sLocalAddress;
		uint16_t iLocalPort;

		uint32_t iThreadId;

		//class IOThread* ptrThreadObj;

		HttpResponseDecoder decoder;

		std::shared_ptr<apie::service::SyncServiceBase> m_ptrDialSyncBase = nullptr;
		std::map<uint64_t, std::shared_ptr<apie::service::SyncServiceBase>> m_pendingSyncSend;

		ClientConnection(const ClientConnection&);
        const ClientConnection &operator = (const ClientConnection&);
    };

}

