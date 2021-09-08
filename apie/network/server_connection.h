#pragma once

#include <stdint.h>
#include <string>
#include <optional>

#include <event2/bufferevent.h>

#include "apie/network/i_poll_events.hpp"
#include "apie/network/object.hpp"
#include "apie/network/command.h"
#include "apie/http/http_request_decoder.h"




namespace apie
{
	struct SetServerSessionAttr;

    class ServerConnection :
        public i_poll_events
    {
    public:
		ServerConnection(uint32_t tid, uint64_t iSerialNum, bufferevent *bev, ProtocolType iType);

		uint64_t getSerialNum();
		uint32_t getTId();

		void readcb();
		void writecb();
		void eventcb(short what);

		void setIp(std::string ip, std::string peerIp);
		void setMaskFlag(uint32_t iFlag);
		uint32_t getMaskFlag();
		std::optional<std::string> getSessionKey();

		void handleSetServerSessionAttr(SetServerSessionAttr* ptrCmd);

		std::string ip();
		std::string peerIp();

		std::string getClientRandom();
		std::string getServerRandom();

		void close(std::string sInfo, uint32_t iCode = 0, uint32_t iActive = 0);
        ~ServerConnection();

	public:
		void handleSend(const char *data, size_t size);
		void handleClose();

		static bool validProtocol(ProtocolType iType);
		static void sendCloseLocalServer(uint64_t iSerialNum);
	private:
		void sendCloseCmd(uint32_t iResult, const std::string& sInfo, uint32_t iActive);

		void readHttp();
		void readPB();
		void recv(MessageInfo info, std::string& requestStr);

    private:
		uint32_t tid_;
		ProtocolType iType;
		uint32_t m_iMaskFlag = 0;
		std::string m_clientRandom;
		std::string m_serverRandom;
		std::optional<std::string>  m_optSessionKey;

		uint64_t iSerialNum;
		bufferevent *bev;

		HttpRequestDecoder decoder;
		std::string sIp;
		std::string sPeerIp;

		ServerConnection(const ServerConnection&) = delete;
        const ServerConnection &operator = (const ServerConnection&) = delete;
    };

}
