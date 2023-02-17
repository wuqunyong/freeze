#include "apie/network/client_connection.h"

#include <sstream>
#include <iostream>

#include "apie/network/ctx.h"

#include "apie/serialization/protocol_head.h"
#include "apie/network/logger.h"
#include "apie/network/address.h"
#include "apie/decompressor/lz4_decompressor_impl.h"
#include "apie/crypto/crypto_utility.h"

#include "apie/service/service_manager.h"
#include "apie/common/protobuf_factory.h"

static const unsigned int MAX_MESSAGE_LENGTH = 16*1024*1024;
static const unsigned int HTTP_BUF_LEN = 8192;

apie::ClientConnection::ClientConnection(integer_t iSerialNum, bufferevent *bev, std::string address, short port, ProtocolType type, uint32_t threadId)
{
	this->iSerialNum = iSerialNum;
	this->bev = bev;
	this->iType = type;
	this->iThreadId = threadId;

	this->SetConnectTo(address,port);

	this->iLocalPort = 0;

	evutil_socket_t fd = bufferevent_getfd(bev);
	struct sockaddr_in addr;
	memset (&addr, 0, sizeof (addr));
	addr.sin_family = AF_INET;

#ifdef WIN32
	int addrlen = sizeof (addr);
#else
	socklen_t addrlen = sizeof (addr);
#endif
	//  Retrieve local port connect is bound to (into addr).
	getsockname(fd, (struct sockaddr*) &addr, &addrlen);

	//this->sLocalAddress = inet_ntoa(addr.sin_addr);

	char buf[100] = { '\0', };
	if (::inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf)) != NULL)
	{
		this->sLocalAddress = buf;
	}

	this->iLocalPort = ntohs(addr.sin_port);

	this->decoder.setConnectSession(this);
}

uint64_t apie::ClientConnection::getSerialNum()
{
	return this->iSerialNum;
}

uint32_t apie::ClientConnection::getTId()
{
	return this->iThreadId;
}

void apie::ClientConnection::close(std::string sInfo, int iCode, int iActive)
{
	std::stringstream ss;
	ss << "close|iSerialNum:" << this->iSerialNum 
		<< "|ip:" << this->sLocalAddress << ":" << this->iLocalPort << " -> " << "peerIp:"<< this->sListenAddress << ":" << this->iListenPort 
		<< "|reason:" << sInfo;
	ASYNC_PIE_LOG("ClientConnection/close", PIE_CYCLE_HOUR, PIE_NOTICE, ss.str().c_str());

	if (this->m_ptrDialSyncBase)
	{
		this->m_ptrDialSyncBase->setException(std::invalid_argument(sInfo));
		this->resetDialSync(nullptr);
	}


	this->sendCloseCmd(iCode, sInfo, iActive);

	apie::event_ns::DispatcherImpl::delClientConnection(this->iSerialNum);
}

void apie::ClientConnection::sendCloseCmd(uint32_t iResult, const std::string& sInfo, uint32_t iActive)
{
	Command cmd;
	cmd.type = Command::client_peer_close;
	cmd.args.client_peer_close.ptrData = new ClientPeerClose();
	cmd.args.client_peer_close.ptrData->iResult = iResult;
	cmd.args.client_peer_close.ptrData->iSerialNum = this->iSerialNum;
	cmd.args.client_peer_close.ptrData->sInfo = sInfo;
	cmd.args.client_peer_close.ptrData->iActive = iActive;
	apie::CtxSingleton::get().getLogicThread()->push(cmd);
}

void apie::ClientConnection::sendConnectResultCmd(uint32_t iResult)
{
	Command cmd;
	cmd.type = Command::dial_result;
	cmd.args.dial_result.ptrData = new DialResult();
	cmd.args.dial_result.ptrData->iResult = iResult;
	cmd.args.dial_result.ptrData->iSerialNum = this->iSerialNum;

	if (this->bev)
	{
		auto iFd = bufferevent_getfd(this->bev);
		auto ptrAddr = network::addressFromFd(iFd);
		if (ptrAddr != nullptr)
		{
			auto ip = network::makeFriendlyAddress(*ptrAddr);
			cmd.args.dial_result.ptrData->sLocalIp = ip;
		}
	}

	if (this->m_ptrDialSyncBase)
	{
		auto ptrData = std::make_shared<service_discovery::ConnectDialResult>();
		ptrData->set_result(iResult);
		ptrData->set_serial_num(iSerialNum);
		this->m_ptrDialSyncBase->getHandler()(ptrData);
		this->resetDialSync(nullptr);
	}
	else
	{

		apie::CtxSingleton::get().getLogicThread()->push(cmd);
	}
}

apie::ClientConnection::~ClientConnection()
{
	if (this->bev != NULL)
	{
		bufferevent_free(this->bev);
		this->bev = NULL;
	}
}

void apie::ClientConnection::readHttp()
{
	char buf[HTTP_BUF_LEN] = { 0 };

	while (true)
	{
		struct evbuffer *input = bufferevent_get_input(this->bev);
		size_t len = evbuffer_get_length(input);
		if (len == 0)
		{
			return;
		}

		size_t minLen = (len > HTTP_BUF_LEN) ? HTTP_BUF_LEN : len;
		evbuffer_remove(input, buf, minLen);

		int result = decoder.execute(buf, minLen);
		if (result != 0)
		{
			return;
		}
	}
}

void apie::ClientConnection::readPB()
{
	while (true)
	{
		struct evbuffer *input = bufferevent_get_input(this->bev);
		size_t len = evbuffer_get_length(input);

		ProtocolHead head;
		size_t iHeadLen = sizeof(ProtocolHead);
		if (len < iHeadLen)
		{
			return;
		}

		size_t iRecvLen = evbuffer_copyout(input, &head, iHeadLen);
		if (iRecvLen < iHeadLen)
		{
			return; // Message incomplete. Waiting for more bytes.
		}

		uint32_t iBodyLen = head.iBodyLen;
		if (iBodyLen > MAX_MESSAGE_LENGTH)
		{
			// Avoid illegal data (too large message) crashing this client.
			std::stringstream ss;
			ss << "active|" << "Message too large: " << iBodyLen << "|Connection closed.";
			this->close(ss.str());
			return;
		}

		if (evbuffer_get_length(input) < iHeadLen + iBodyLen) {
			return; // Message incomplete. Waiting for more bytes.
		}
		evbuffer_drain(input, iHeadLen);

		//char* pBuf = (char*)malloc(iBodyLen + 1); // Add space for trailing '\0'.

		//evbuffer_remove(input, pBuf, iBodyLen);
		//pBuf[iBodyLen] = '\0';

		//std::string sBody(pBuf, iBodyLen);
		//free(pBuf);

		std::string sBody;
		sBody.resize(iBodyLen, '\0');
		evbuffer_remove(input, &sBody[0], iBodyLen);

		do
		{
			if (head.iFlags & PH_CRYPTO)
			{
				if (this->getSessionKey().has_value())
				{
					sBody = apie::crypto::Utility::decode_rc4(this->getSessionKey().value(), sBody);
				}
				else
				{
					std::stringstream ss;
					ss << "active|" << "iOpcode:" << head.iOpcode << "|PH_CRYPTO error|Connection closed.";
					this->close(ss.str());
					return;
				}
			}

			if (head.iFlags & PH_COMPRESSED)
			{
				decompressor::LZ4DecompressorImpl decompressor;
				auto optDate = decompressor.decompress(sBody);
				if (!optDate.has_value())
				{
					//Error
					std::stringstream ss;
					ss << "active|" << "iOpcode:" << head.iOpcode << "|PH_COMPRESSED error|Connection closed.";
					this->close(ss.str());
					return;
				}

				sBody = optDate.value();
			}

			MessageInfo info;
			info.iSessionId = this->iSerialNum;
			info.iSeqNum = head.iSeqNum;
			info.iOpcode = head.iOpcode;
			info.iConnetionType = ConnetionType::CT_CLIENT;
			this->recv(info, sBody);

		} while (false);


		size_t iCurLen = evbuffer_get_length(input);
		if (iCurLen < iHeadLen)
		{
			return;
		}
	}
}

void apie::ClientConnection::readPBMsgHead()
{
	while (true)
	{
		struct evbuffer* input = bufferevent_get_input(this->bev);
		size_t len = evbuffer_get_length(input);

		PBMsgHead head;
		size_t iHeadLen = sizeof(PBMsgHead);
		if (len < iHeadLen)
		{
			return;
		}

		size_t iRecvLen = evbuffer_copyout(input, &head, iHeadLen);
		if (iRecvLen < iHeadLen)
		{
			return; // Message incomplete. Waiting for more bytes.
		}

		uint32_t iTotalLen = head.iSize;
		if (iTotalLen > MAX_MESSAGE_LENGTH)
		{
			// Avoid illegal data (too large message) crashing this client.
			std::stringstream ss;
			ss << "active|" << "Message too large: " << iTotalLen << "|Connection closed.";
			this->close(ss.str());
			return;
		}

		if (iTotalLen < iHeadLen)
		{
			std::stringstream ss;
			ss << "active|" << "iTotalLen < iHeadLen: " << iTotalLen << "|Connection closed.";
			this->close(ss.str());
			return;
		}

		uint32_t iBodyLen = iTotalLen - iHeadLen;
		if (evbuffer_get_length(input) < iTotalLen) 
		{
			return; // Message incomplete. Waiting for more bytes.
		}
		evbuffer_drain(input, iHeadLen);


		std::string sBody;
		sBody.resize(iBodyLen, '\0');
		evbuffer_remove(input, &sBody[0], iBodyLen);

		uint32_t iOpcode = MergeOpcode(head.iType, head.iCmd);

		MessageInfo info;
		info.iSessionId = this->iSerialNum;
		info.iSeqNum = head.idSeq;
		info.iOpcode = iOpcode;
		info.iConnetionType = ConnetionType::CT_CLIENT;
		info.iCodec = this->iType;
		this->recv(info, sBody);

		size_t iCurLen = evbuffer_get_length(input);
		if (iCurLen < iHeadLen)
		{
			return;
		}
	}
}

void apie::ClientConnection::readPBMsgUser()
{
	while (true)
	{
		struct evbuffer* input = bufferevent_get_input(this->bev);
		size_t len = evbuffer_get_length(input);

		PBMsgUser head;
		size_t iHeadLen = sizeof(PBMsgUser);
		if (len < iHeadLen)
		{
			return;
		}

		size_t iRecvLen = evbuffer_copyout(input, &head, iHeadLen);
		if (iRecvLen < iHeadLen)
		{
			return; // Message incomplete. Waiting for more bytes.
		}

		uint32_t iTotalLen = head.iSize;
		if (iTotalLen > MAX_MESSAGE_LENGTH)
		{
			// Avoid illegal data (too large message) crashing this client.
			std::stringstream ss;
			ss << "active|" << "Message too large: " << iTotalLen << "|Connection closed.";
			this->close(ss.str());
			return;
		}

		if (iTotalLen < iHeadLen)
		{
			std::stringstream ss;
			ss << "active|" << "iTotalLen < iHeadLen: " << iTotalLen << "|Connection closed.";
			this->close(ss.str());
			return;
		}

		uint32_t iBodyLen = iTotalLen - iHeadLen;
		if (evbuffer_get_length(input) < iTotalLen)
		{
			return; // Message incomplete. Waiting for more bytes.
		}
		evbuffer_drain(input, iHeadLen);


		std::string sBody;
		sBody.resize(iBodyLen, '\0');
		evbuffer_remove(input, &sBody[0], iBodyLen);

		uint32_t iOpcode = MergeOpcode(head.iType, head.iCmd);

		MessageInfo info;
		info.iSessionId = this->iSerialNum;
		info.iSeqNum = head.idSeq;
		info.iOpcode = iOpcode;
		info.iConnetionType = ConnetionType::CT_CLIENT;
		info.iCodec = this->iType;
		info.iUserId = head.iUserId;
		info.s_idSeq = head.s_idSeq;
		this->recv(info, sBody);

		size_t iCurLen = evbuffer_get_length(input);
		if (iCurLen < iHeadLen)
		{
			return;
		}
	}
}

void apie::ClientConnection::recv(MessageInfo info, std::string& requestStr)
{
	auto optionalData = apie::service::ServiceHandlerSingleton::get().client.getType(info.iOpcode);
	if (!optionalData)
	{
		PBForward *itemObjPtr = new PBForward;
		itemObjPtr->type = ConnetionType::CT_CLIENT;
		itemObjPtr->info = info;
		itemObjPtr->sMsg = requestStr;

		Command command;
		command.type = Command::pb_forward;
		command.args.pb_forward.ptrData = itemObjPtr;

		auto ptrLogic = apie::CtxSingleton::get().getLogicThread();
		if (ptrLogic == nullptr)
		{
			delete itemObjPtr;

			std::stringstream ss;
			ss << "getLogicThread null|iSerialNum:" << info.iSessionId << "|iOpcode:" << info.iOpcode;
			ASYNC_PIE_LOG("ClientConnection/recv", PIE_CYCLE_DAY, PIE_ERROR, "%s", ss.str().c_str());
			return;
		}

		ptrLogic->push(command);
		return;
	}

	std::string sType = optionalData.value();
	auto ptrMsg = apie::message::ProtobufFactory::createMessage(sType);
	if (ptrMsg == nullptr)
	{
		std::stringstream ss;
		ss << "createMessage error|iSerialNum:" << info.iSessionId << "|iOpcode:" << info.iOpcode << "|sType:" << sType;
		ASYNC_PIE_LOG("ClientConnection/recv", PIE_CYCLE_DAY, PIE_ERROR, "%s", ss.str().c_str());
		return;
	}

	std::shared_ptr<::google::protobuf::Message> newMsg(ptrMsg);
	bool bResult = newMsg->ParseFromString(requestStr);
	if (!bResult)
	{
		std::stringstream ss;
		ss << "ParseFromString error|iSerialNum:" << info.iSessionId << "|iOpcode:" << info.iOpcode << "|sType:" << sType;
		ASYNC_PIE_LOG("ClientConnection/recv", PIE_CYCLE_DAY, PIE_ERROR, "%s", ss.str().c_str());
		return;
	}

	//newMsg->PrintDebugString();

	bool bSync = this->triggerSyncHandler(info.iSeqNum, newMsg);
	if (bSync)
	{
		return;
	}


	PBRequest *itemObjPtr = new PBRequest;
	itemObjPtr->type = ConnetionType::CT_CLIENT;
	itemObjPtr->info = info;
	itemObjPtr->ptrMsg = newMsg;

	Command command;
	command.type = Command::pb_reqeust;
	command.args.pb_reqeust.ptrData = itemObjPtr;

	auto ptrLogic = apie::CtxSingleton::get().getLogicThread();
	if (ptrLogic == nullptr)
	{
		delete itemObjPtr;

		std::stringstream ss;
		ss << "getLogicThread null|iSerialNum:" << info.iSessionId << "|iOpcode:" << info.iOpcode;
		ASYNC_PIE_LOG("ClientConnection/recv", PIE_CYCLE_DAY, PIE_ERROR, "%s", ss.str().c_str());
		return;
	}
	ptrLogic->push(command);
}

void apie::ClientConnection::readcb()
{
	switch (this->iType)
	{
	case ProtocolType::PT_HTTP:
	{
		this->readHttp();
		break;
	}
	case ProtocolType::PT_PB:
	{
		this->readPB();
		break;
	}
	case ProtocolType::PT_PBMsgHead:
	{
		this->readPBMsgHead();
		break;
	}
	case ProtocolType::PT_PBMsgUser:
	{
		this->readPBMsgUser();
		break;
	}
	default:
		break;
	}
}

void apie::ClientConnection::writecb()
{

}

void apie::ClientConnection::eventcb(short what)
{
	if (what & BEV_EVENT_EOF)
	{
		std::stringstream ss;
		ss << "passive|" << what << "|BEV_EVENT_EOF" << "|Connection closed.";
		//printf("Connection closed.\n");
		this->close(ss.str());
	} 
	else if (what & BEV_EVENT_ERROR)
	{
		std::stringstream ss;
		ss << "passive|" << what << "|BEV_EVENT_ERROR" << "|Got an error on the connection|" << strerror(errno);
		//printf("Got an error on the connection: %s\n",strerror(errno));/*XXX win32*/
		this->close(ss.str(),BEV_EVENT_ERROR);
	} 
	else if (what & BEV_EVENT_CONNECTED) 
	{
		this->sendConnectResultCmd(0);
	}
	else if (what & BEV_EVENT_TIMEOUT)
	{
		std::stringstream ss;
		ss << "active|" << what << "|BEV_EVENT_TIMEOUT";
		this->close(ss.str());
	}
	else
	{
		std::stringstream ss;
		ss << "passive|" << what << "|OTHER";
		this->close(ss.str());
	}
	
}


void apie::ClientConnection::SetConnectTo(const std::string& sAddress, uint16_t iPort)
{
	this->sListenAddress = sAddress;
	this->iListenPort = iPort;
}

void apie::ClientConnection::handleSetClientSessionAttr(SetClientSessionAttr* ptrCmd)
{
	if (ptrCmd == nullptr)
	{
		return;
	}

	if (ptrCmd->optKey.has_value())
	{
		m_optSessionKey = ptrCmd->optKey.value();
	}
}

std::optional<std::string> apie::ClientConnection::getSessionKey()
{
	return m_optSessionKey;
}

void apie::ClientConnection::handleSend(const char *data, size_t size)
{
	if (NULL != this->bev)
	{
		int rc = bufferevent_write(this->bev, data, size);
		if (rc != 0)
		{
			PIE_FMT_LOG("Exception/Exception",PIE_CYCLE_DAY,PIE_ERROR,"ClientConnection|handleSend Error:{}|{}|{}", rc, size, data);
		}
		else
		{
			//std::stringstream ss;
			//ss << "Session/" << this->iSerialNum;
			//std::string sFile = ss.str();
			//pieLog(sFile.c_str(),PIE_CYCLE_DAY,PIE_DEBUG,"ClientConnection|handleSend:data|%s|size|%d",data,size);
		}

	}
}

void apie::ClientConnection::handleClose()
{
	std::stringstream ss;
	ss << "ClientConnection|active|" << "call By C_closeSocket";
	this->close(ss.str(),0,1);
}

void apie::ClientConnection::resetDialSync(std::shared_ptr<apie::service::SyncServiceBase> ptrDialSyncBase)
{
	m_ptrDialSyncBase = ptrDialSyncBase;
}

bool apie::ClientConnection::addSyncSend(uint64_t iId, std::shared_ptr<apie::service::SyncServiceBase> ptrSync)
{
	if (ptrSync == nullptr)
	{
		return false;
	}

	auto findIte = m_pendingSyncSend.find(iId);
	if (findIte == m_pendingSyncSend.end())
	{
		m_pendingSyncSend[iId] = ptrSync;
		return true;
	}

	return false;
}

bool apie::ClientConnection::triggerSyncHandler(uint64_t iId, const std::shared_ptr<::google::protobuf::Message>& response)
{
	if (iId == 0)
	{
		return false;
	}

	auto findIte = m_pendingSyncSend.find(iId);
	if (findIte == m_pendingSyncSend.end())
	{
		return false;
	}

	if (findIte->second)
	{
		findIte->second->getHandler()(response);
	}
	m_pendingSyncSend.erase(findIte);

	return true;
}

static void client_readcb(struct bufferevent *bev, void *arg)
{
	apie::ClientConnection *ptrSession = (apie::ClientConnection *)arg;
	ptrSession->readcb();
}

static void client_writecb(struct bufferevent *bev, void *arg)
{
	apie::ClientConnection *ptrSession = (apie::ClientConnection *)arg;
	ptrSession->writecb();
}

static void client_eventcb(struct bufferevent *bev, short what, void *arg)
{
	apie::ClientConnection *ptrSession = (apie::ClientConnection *)arg;
	ptrSession->eventcb(what);
}

std::shared_ptr<apie::ClientConnection> apie::ClientConnection::createClient(uint32_t threadId, struct event_base *base, DialParameters* ptrDial)
{
	std::shared_ptr<apie::ClientConnection> ptrSharedClient(nullptr);

	uint64_t iSerialNum = ptrDial->iCurSerialNum;
	uint16_t iPort = ptrDial->iPort;
	ProtocolType iCodecType =  ptrDial->iCodecType;
	const char* ip = ptrDial->sIp.c_str();

	uint32_t iResult = 0;
	struct bufferevent * bev = NULL;

	do
	{
		struct sockaddr_in s;
		memset(&s, 0, sizeof(struct sockaddr_in));
		s.sin_family = AF_INET;
		s.sin_port = htons(iPort);
		//s.sin_addr.s_addr = inet_addr(ip);
		::inet_pton(AF_INET, ip, &s.sin_addr);

		if (s.sin_addr.s_addr == INADDR_NONE)
		{
			apie::network::getInAddr(&s.sin_addr, ip);
		}

		bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
		if (NULL == bev)
		{
			iResult = 2;
			break;
		}

		if (bufferevent_socket_connect(bev, (struct sockaddr*)&s, sizeof(struct sockaddr)) < 0)
		{
			fprintf(stderr, "bufferevent_socket_connect return <0 ! errno=%d,%s.", errno, strerror(errno));
			bufferevent_free(bev);
			bev = NULL;

			iResult = 3;
			break;
		}

		evutil_socket_t fd = bufferevent_getfd(bev);
		if (-1 == fd)
		{
			fprintf(stderr, "bufferevent_getfd return -1 ! ");

			bufferevent_free(bev);
			bev = NULL;
			iResult = 4;
			break;
		}

		//  Disable Nagle's algorithm.
		int flag = 1;
		int rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
		if (rc != 0)
		{
			PIE_FMT_LOG("Exception/Exception", PIE_CYCLE_DAY, PIE_WARNING, "processActiveConnect|setsockopt|TCP_NODELAY:{}|{}:{}", rc, ip, iPort);
		}
		//assert(rc == 0);

		int on = 1;
		rc = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&on, sizeof(on));
		if (rc != 0)
		{
			PIE_FMT_LOG("Exception/Exception", PIE_CYCLE_DAY, PIE_WARNING, "processActiveConnect|setsockopt|SO_KEEPALIVE:{}|{}:{}", rc, ip, iPort);
		}
		//assert(rc == 0);

		std::string sAddress(ip);
		ClientConnection *ptrConnectSession = new (std::nothrow) ClientConnection(iSerialNum, bev, sAddress, iPort, iCodecType, threadId);
		if (NULL == ptrConnectSession)
		{
			fprintf(stderr, "New ClientConnection Error!");
			bufferevent_free(bev);
			bev = NULL;
			iResult = 5;
			break;
		}

		ptrSharedClient.reset(ptrConnectSession);
		apie::event_ns::DispatcherImpl::addClientConnection(ptrSharedClient);


		bufferevent_setcb(bev, client_readcb, client_writecb, client_eventcb, ptrConnectSession);
		bufferevent_enable(bev, EV_READ | EV_WRITE);

		struct timeval tv_read;
		tv_read.tv_sec = 600;
		tv_read.tv_usec = 0;
		struct timeval tv_write;
		tv_write.tv_sec = 600;
		tv_write.tv_usec = 0;
		bufferevent_set_timeouts(bev, &tv_read, &tv_write);
	} while (false);

	if (0 != iResult)
	{
		Command cmd;
		cmd.type = Command::dial_result;
		cmd.args.dial_result.ptrData = new DialResult();
		cmd.args.dial_result.ptrData->iResult = iResult;
		cmd.args.dial_result.ptrData->iSerialNum = iSerialNum;

		if (ptrDial->mode == DIAL_MODE::DM_SYNC)
		{
			if (ptrDial->ptrSyncBase)
			{
				auto ptrData = std::make_shared<service_discovery::ConnectDialResult>();
				ptrData->set_result(iResult);
				ptrData->set_serial_num(iSerialNum);
				ptrDial->ptrSyncBase->getHandler()(ptrData);
			}
			else
			{
				//TODO
			}
		} 
		else
		{
			apie::CtxSingleton::get().getLogicThread()->push(cmd);
		}
	}

	if (ptrDial->mode == DIAL_MODE::DM_SYNC)
	{
		ptrSharedClient->resetDialSync(ptrDial->ptrSyncBase);
	}

	return ptrSharedClient;
}