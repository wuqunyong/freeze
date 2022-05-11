#include "apie/network/client_proxy.h"

#include <assert.h>
#include <iostream>
#include <sstream>


#include "apie/network/ctx.h"
#include "apie/network/command.h"
#include "apie/network/logger.h"
#include "apie/network/output_stream.h"
#include "apie/event/dispatcher_impl.h"
#include "apie/common/enum_to_int.h"


using namespace apie;

std::mutex ClientProxy::m_sync;
std::map<uint64_t, std::shared_ptr<ClientProxy>> ClientProxy::m_clientProxy;

ClientProxy::ClientProxy() 
{
	this->m_tag = 0xbadcafe0;
	this->m_curSerialNum = 0;
	this->m_port = 0;
	this->m_codecType = ProtocolType::PT_PB;

	this->m_hadEstablished = CONNECT_CLOSE;
	this->m_reconnectTimes = 0;
	this->m_tId = 0;

	auto reconnectCb = [this](){
		std::stringstream ss;
		ss << "reconnect|SerialNum:" << this->m_curSerialNum << "|ip:" << this->m_localIp << " -> " << "peerIp:" << this->m_ip << ":" << this->m_port;
		ASYNC_PIE_LOG("ClientProxy", PIE_CYCLE_DAY, PIE_NOTICE, "reconnect|m_reconnectTimes:%d|%s", m_reconnectTimes, ss.str().c_str());

		this->reconnect(); 
		if (this->isConnectted())
		{
			return;
		}
		this->addReconnectTimer(3000);
	};
	this->m_reconnectTimer = apie::CtxSingleton::get().getLogicThread()->dispatcher().createTimer(reconnectCb);

	auto heartbeatCb = [this]() {
		if (this->m_heartbeatCb)
		{
			this->m_heartbeatCb(this);
		}
	};
	this->m_heartbeatTimer = apie::CtxSingleton::get().getLogicThread()->dispatcher().createTimer(heartbeatCb);
}

ClientProxy::~ClientProxy()
{
	this->disableReconnectTimer();
	this->disableHeartbeatTimer();

	this->m_tag = 0xdeadbeef;
}

bool ClientProxy::checkTag()
{
	return this->m_tag == 0xbadcafe0;
}

uint32_t ClientProxy::getTId()
{
	return m_tId;
}

int ClientProxy::connect(const std::string& ip, uint16_t port, ProtocolType type, uint32_t maskFlag, HandleConnectCB cb)
{
	if (this->m_curSerialNum != 0)
	{
		return opcodes::SC_ClientProxy_SerialNumNotEqualZero;
	}

	this->m_curSerialNum = generatorId();
	this->m_ip = ip;
	this->m_port = port;
	this->m_codecType = type;
	this->m_maskFlag = maskFlag;
	this->m_cb = cb;

	auto ptrProxy = shared_from_this();
	ClientProxy::registerClientProxy(ptrProxy);

	return this->sendConnect();
}

bool ClientProxy::syncConnect(const std::string& ip, uint16_t port, ProtocolType type, uint32_t maskFlag, HandleConnectCB cb)
{
	if (this->m_curSerialNum != 0)
	{
		return false;
	}

	this->m_curSerialNum = generatorId();
	this->m_ip = ip;
	this->m_port = port;
	this->m_codecType = type;
	this->m_maskFlag = maskFlag;
	this->m_cb = cb;

	auto ptrProxy = shared_from_this();
	ClientProxy::registerClientProxy(ptrProxy);

	auto future = this->syncSendConnect();
	if (!future.valid()) 
	{
		return false;
	}

	std::chrono::seconds timeout_s(10);
	auto status = future.wait_for(timeout_s);
	if (status == std::future_status::ready) 
	{
		try
		{
			auto pbObj = future.get();
			if (pbObj->result() == 0)
			{
				this->m_reconnectTimes = 0;
				this->m_hadEstablished = CONNECT_ESTABLISHED;

				return true;
			}

			return false;
		}
		catch (const std::exception& e)
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
}

void ClientProxy::resetConnect(const std::string& ip, uint16_t port, ProtocolType type)
{
	this->m_ip = ip;
	this->m_port = port;
	this->m_codecType = type;
}

int ClientProxy::reconnect()
{
	if (this->m_curSerialNum == 0)
	{
		return opcodes::SC_ClientProxy_SerialNumEqualZero;
	}

	if (this->m_hadEstablished == CONNECT_ESTABLISHED)
	{
		return opcodes::SC_ClientProxy_Established;
	}

	this->m_reconnectTimes++;
	return this->sendConnect();
}

void ClientProxy::addReconnectTimer(uint64_t interval)
{
	this->m_reconnectTimer->enableTimer(std::chrono::milliseconds(interval));
}

void ClientProxy::disableReconnectTimer()
{
	this->m_reconnectTimer->disableTimer();
}

void ClientProxy::setHeartbeatCb(HeartbeatCB cb)
{
	this->m_heartbeatCb = cb;
}

void ClientProxy::addHeartbeatTimer(uint64_t interval)
{
	this->m_heartbeatTimer->enableTimer(std::chrono::milliseconds(interval));
}

void ClientProxy::disableHeartbeatTimer()
{
	this->m_heartbeatTimer->disableTimer();
}

uint64_t ClientProxy::getSerialNum()
{
	return this->m_curSerialNum;
}

std::string ClientProxy::getCurSerialNumToStr()
{
	std::stringstream ss;
	ss << this->m_curSerialNum;

	return ss.str();
}

ProtocolType ClientProxy::getCodecType()
{
	return this->m_codecType;
}

std::string ClientProxy::getHosts()
{
	std::stringstream ss;
	ss << this->m_ip << ":" << this->m_port;

	return ss.str();
}

bool ClientProxy::isConnectted()
{
	return this->m_hadEstablished == CONNECT_ESTABLISHED;
}

void ClientProxy::setHadEstablished(uint32_t value)
{
	this->m_hadEstablished = value;
}

event_ns::TimerPtr& ClientProxy::reconnectTimer()
{
	return m_reconnectTimer;
}

void ClientProxy::setLocalIp(const std::string& ip)
{
	m_localIp = ip;
}

int64_t ClientProxy::getUserId()
{
	return m_iUserId;
}

void ClientProxy::setUserId(int64_t iUserId)
{
	m_iUserId = iUserId;
}

int32_t ClientProxy::sendMsg(uint32_t iOpcode, const ::google::protobuf::Message& msg)
{
	if (this->m_hadEstablished != CONNECT_ESTABLISHED)
	{
		return opcodes::SC_ClientProxy_NotEstablished;
	}

	m_sequenceNumber++;

	switch (m_codecType)
	{
	case apie::ProtocolType::PT_PB:
	{
		if (this->m_maskFlag == 0)
		{
			MessageInfo info;
			info.iSessionId = this->m_curSerialNum;
			info.iSeqNum = m_sequenceNumber;
			info.iOpcode = iOpcode;
			info.iConnetionType = apie::ConnetionType::CT_CLIENT;
			apie::network::OutputStream::sendProtobufMsgImpl(info, msg);
		}
		else
		{
			MessageInfo info;
			info.iSessionId = this->m_curSerialNum;
			info.iSeqNum = m_sequenceNumber;
			info.iOpcode = iOpcode;
			info.iConnetionType = apie::ConnetionType::CT_CLIENT;
			info.setFlags(this->m_maskFlag);
			apie::network::OutputStream::sendMsgWithFlag(info, msg);
		}
		break;
	}
	case apie::ProtocolType::PT_PBMsgHead:
	{
		MessageInfo info;
		info.iSessionId = this->m_curSerialNum;
		info.iSeqNum = m_sequenceNumber;
		info.iOpcode = iOpcode;
		info.iConnetionType = apie::ConnetionType::CT_CLIENT;
		apie::network::OutputStream::sendPBMsgHead(info, msg);
		break;
	}
	case apie::ProtocolType::PT_PBMsgUser:
	{
		MessageInfo info;
		info.iSessionId = this->m_curSerialNum;
		info.iSeqNum = m_sequenceNumber;
		info.iOpcode = iOpcode;
		info.iConnetionType = apie::ConnetionType::CT_CLIENT;

		info.iUserId = this->getUserId();
		apie::network::OutputStream::sendPBMsgUser(info, msg);
		break;
	}
	default:
	{
		std::stringstream ss;
		ss << "invalid m_codecType: " << toUnderlyingType(m_codecType);
		throw std::invalid_argument(ss.str());
		break;
	}
	}

	return 0;
}


uint32_t ClientProxy::getReconnectTimes()
{
	return this->m_reconnectTimes;
}

void ClientProxy::onConnect(uint32_t iResult)
{
	std::stringstream ss;
	ss << "recv|SerialNum:" << this->m_curSerialNum << "|ip:" << this->m_localIp << " -> "<< "peerIp:" << this->m_ip << ":" << this->m_port << ",iResult:" << iResult;
	ASYNC_PIE_LOG("ClientProxy/onConnect", PIE_CYCLE_HOUR, PIE_NOTICE, ss.str().c_str());

	if (iResult == 0)
	{
		this->m_reconnectTimes = 0;
		this->m_hadEstablished = CONNECT_ESTABLISHED;
	}

	bool bContinue = false;
	if (m_cb)
	{
		bContinue = m_cb(this, iResult);
	}

	if (iResult != 0)
	{
		if (bContinue)
		{
			return;
		}

		this->close();
	}
}

void ClientProxy::onPassiveClose(uint32_t iResult, const std::string& sInfo, uint32_t iActiveClose)
{
	std::stringstream ss;
	ss << "recv|SerialNum:" << this->m_curSerialNum << ",ip:" << this->m_ip << ",port:" << this->m_port 
		<< ",iResult:" << iResult << ",sInfo:" << sInfo << ",iActiveClose:" << iActiveClose;
	ASYNC_PIE_LOG("ClientProxy/onPassiveClose", PIE_CYCLE_HOUR, PIE_NOTICE, ss.str().c_str());

	this->m_hadEstablished = CONNECT_CLOSE;

	this->close();
}

void ClientProxy::onActiveClose()
{
	this->sendClose();

	this->m_hadEstablished = CONNECT_CLOSE;
	this->close();
}

int ClientProxy::sendConnect()
{
	auto *ptr = new DialParameters;
	if (NULL == ptr)
	{
		return opcodes::SC_ClientProxy_BadAlloc;
	}
	ptr->mode = DIAL_MODE::DM_ASYNC;
	ptr->sIp = this->m_ip;
	ptr->iPort = this->m_port;
	ptr->iCodecType = this->m_codecType;
	ptr->iCurSerialNum = this->m_curSerialNum;
	ptr->ptrSyncBase = nullptr;

	Command cmd;
	cmd.type = Command::dial;
	cmd.args.dial.ptrData = ptr;

	if (m_tId == 0)
	{
		auto ptrThread = apie::CtxSingleton::get().chooseIOThread();
		if (ptrThread == NULL)
		{
			delete ptr;
			return opcodes::SC_ClientProxy_NoIOThread;
		}
		m_tId = ptrThread->getTId();
	}

	auto ptrIOThread = apie::CtxSingleton::get().getThreadById(m_tId);
	if (ptrIOThread == NULL)
	{
		delete ptr;
		return opcodes::SC_ClientProxy_NoIOThread;
	}

	ptrIOThread->push(cmd);

	std::stringstream ss;
	ss << "send|SerialNum:" << this->m_curSerialNum << ",ip:" << this->m_ip << ",port:" << this->m_port
		<<",reconnectTimes:" << this->m_reconnectTimes;
	ASYNC_PIE_LOG("ClientProxy/connect", PIE_CYCLE_HOUR, PIE_NOTICE, ss.str().c_str());

	return 0;
}

std::shared_future<std::shared_ptr<service_discovery::ConnectDialResult>> ClientProxy::syncSendConnect()
{
	std::shared_future<std::shared_ptr<service_discovery::ConnectDialResult>> invalidResult;

	auto* ptr = new DialParameters;
	if (NULL == ptr)
	{
		return invalidResult;
	}

	auto ptrSync = std::make_shared<apie::service::SyncService<service_discovery::ConnectDialResult>>();
	ptr->mode = DIAL_MODE::DM_SYNC;
	ptr->sIp = this->m_ip;
	ptr->iPort = this->m_port;
	ptr->iCodecType = this->m_codecType;
	ptr->iCurSerialNum = this->m_curSerialNum;
	ptr->ptrSyncBase = ptrSync;

	Command cmd;
	cmd.type = Command::dial;
	cmd.args.dial.ptrData = ptr;

	if (m_tId == 0)
	{
		auto ptrThread = apie::CtxSingleton::get().chooseIOThread();
		if (ptrThread == NULL)
		{
			delete ptr;
			return invalidResult;
		}
		m_tId = ptrThread->getTId();
	}

	auto ptrIOThread = apie::CtxSingleton::get().getThreadById(m_tId);
	if (ptrIOThread == NULL)
	{
		delete ptr;
		return invalidResult;
	}

	ptrIOThread->push(cmd);

	std::stringstream ss;
	ss << "send|SerialNum:" << this->m_curSerialNum << ",ip:" << this->m_ip << ",port:" << this->m_port
		<< ",reconnectTimes:" << this->m_reconnectTimes;
	ASYNC_PIE_LOG("ClientProxy/sync_connect", PIE_CYCLE_HOUR, PIE_NOTICE, ss.str().c_str());


	return ptrSync->getFuture();
}

void ClientProxy::sendClose()
{
	apie::CloseLocalClient *ptr = new apie::CloseLocalClient;
	ptr->iSerialNum = this->m_curSerialNum;

	Command cmd;
	cmd.type = Command::close_local_client;
	cmd.args.close_local_client.ptrData = ptr;

	auto ptrIOThread = apie::CtxSingleton::get().getThreadById(m_tId);
	if (ptrIOThread == NULL)
	{
		delete ptr;
		return;
	}
	ptrIOThread->push(cmd);
}

void ClientProxy::close()
{
	std::stringstream ss;
	ss << "close|SerialNum:" << this->m_curSerialNum << "|ip:" << this->m_localIp << " -> "<< "peerIp:" << this->m_ip << ":" << this->m_port;
	ASYNC_PIE_LOG("ClientProxy/close", PIE_CYCLE_HOUR, PIE_NOTICE, ss.str().c_str());

	ClientProxy::unregisterClientProxy(this->m_curSerialNum);
}

uint64_t ClientProxy::generatorId()
{
	uint64_t iSerialNum = apie::event_ns::DispatcherImpl::generatorSerialNum();
	return iSerialNum;
}

void ClientProxy::onRecvPackage(uint64_t iSerialNum, ::google::protobuf::Message* ptrMsg)
{
	std::stringstream ss;
	ss << "iSerialNum:" << iSerialNum << ",Message:" << ptrMsg->ShortDebugString();
	ASYNC_PIE_LOG("ClientProxy/onRecvPackage", PIE_CYCLE_HOUR, PIE_NOTICE, ss.str().c_str());
}

bool ClientProxy::registerClientProxy(std::shared_ptr<ClientProxy> ptrClient)
{
	std::lock_guard<std::mutex> guard(m_sync);
	m_clientProxy[ptrClient->getSerialNum()] = ptrClient;
	return true;
}

void ClientProxy::unregisterClientProxy(uint64_t iSerialNum)
{
	std::lock_guard<std::mutex> guard(m_sync);
	m_clientProxy.erase(iSerialNum);
}

std::shared_ptr<ClientProxy> ClientProxy::findClientProxy(uint64_t iSerialNum)
{
	std::lock_guard<std::mutex> guard(m_sync);
	auto findIte = m_clientProxy.find(iSerialNum);
	if (findIte == m_clientProxy.end())
	{
		return nullptr;
	}

	return findIte->second;
}

void ClientProxy::clearAllClientProxy()
{
	std::lock_guard<std::mutex> guard(m_sync);
	m_clientProxy.clear();
}

std::shared_ptr<ClientProxy> ClientProxy::createClientProxy()
{
	return std::make_shared<ClientProxy>();
}