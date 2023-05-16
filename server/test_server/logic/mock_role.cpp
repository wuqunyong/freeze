#include "logic/mock_role.h"

#include <functional>

#include "logic/test_server.h"

namespace apie {


std::map<std::string, std::map<std::string, MockRole::HandlerCb>> MockRole::m_cmdHandler;
std::map<uint32_t, MockRole::HandleResponseCB> MockRole::m_dataHandler;
std::map<uint32_t, std::string> MockRole::s_pbReflect;

MockRole::MockRole(uint64_t iIggId) :
	m_iIggId(iIggId)
{
}

MockRole::~MockRole()
{
	if (this->m_cmdTimer)
	{
		this->m_cmdTimer->disableTimer();
	}

	if (this->m_clientProxy)
	{
		APieGetModule<apie::TestServerMgr>()->removeSerialNum(this->m_clientProxy->getSerialNum());

		this->m_clientProxy->onActiveClose();
	}
}

void MockRole::setUp()
{
	if (m_bInit)
	{
		return;
	}

	m_bInit = true;
	APieGetModule<apie::TestServerMgr>()->addSerialNumRole(this->m_clientProxy->getSerialNum(), m_iIggId);

	this->addResponseHandler(pb::core::OP_AccountLoginResponse, &MockRole::handleAccountLogin);
	this->addResponseHandler(pb::core::OP_ClientLoginResponse, &MockRole::handleClientLogin);

	//µÇÂ¼·µ»Ø

	this->processCmd();
}

void MockRole::tearDown()
{

}

void MockRole::start()
{
	std::string ip = apie::CtxSingleton::get().getConfigs()->login_server.address;
	uint16_t port = apie::CtxSingleton::get().getConfigs()->login_server.port_value;
	uint16_t type = apie::CtxSingleton::get().getConfigs()->login_server.type;
	uint32_t maskFlag = apie::CtxSingleton::get().getConfigs()->login_server.mask_flag;

	m_clientProxy = apie::ClientProxy::createClientProxy();
	m_clientProxy->setUserId(this->m_iIggId);

	std::weak_ptr<MockRole> ptrSelf = this->shared_from_this();

	auto connectCb = [ptrSelf](apie::ClientProxy* ptrClient, uint32_t iResult) mutable {
		if (iResult == 0)
		{
			auto ptrShared = ptrSelf.lock();
			if (ptrShared)
			{
				ptrShared->setUp();
			}
		}
		return true;
	};
	m_clientProxy->connect(ip, port, static_cast<apie::ProtocolType>(type), maskFlag, connectCb);
	m_clientProxy->addReconnectTimer(1000);
	m_target = CT_Login;

	auto cmdCb = [ptrSelf]() mutable {
		auto ptrShared = ptrSelf.lock();
		if (ptrShared)
		{
			ptrShared->processCmd();
			ptrShared->addTimer(100);
		}
	};
	this->m_cmdTimer = apie::CtxSingleton::get().getLogicThread()->dispatcher().createTimer(cmdCb);
	this->addTimer(100);
}

uint64_t MockRole::getIggId()
{
	return m_iIggId;
}

std::shared_ptr<ClientProxy> MockRole::getClientProxy()
{
	return m_clientProxy;
}

void MockRole::setClientProxy(std::shared_ptr<ClientProxy> ptrProxy)
{
	m_clientProxy = ptrProxy;
}

void MockRole::processCmd()
{
	if (!m_bInit)
	{
		return;
	}

	if (!m_clientProxy)
	{
		return;
	}

	if (!m_clientProxy->isConnectted())
	{
		return;
	}

	this->sendKeepAlive();

	if (m_configCmd.empty())
	{
		return;
	}

	while (m_iCurIndex < m_configCmd.size())
	{
		if (m_bPauseProcess)
		{
			break;
		}

		auto iOldIndex = m_iCurIndex;

		auto& msg = m_configCmd[m_iCurIndex];
		this->handleMsg(msg);

		if (iOldIndex != m_iCurIndex)
		{
			break;
		}

		m_iCurIndex++;
	}
}

void MockRole::addTimer(uint64_t interval)
{
	this->m_cmdTimer->enableTimer(std::chrono::milliseconds(interval));
}

void MockRole::disableCmdTimer()
{
	this->m_cmdTimer->disableTimer();
}

void MockRole::handleMsg(::pubsub::TEST_CMD& msg)
{
	auto sModule = msg.module_name();
	auto sCmd = msg.cmd();
	auto handler = this->findHandler(sModule, sCmd);
	if (handler == nullptr)
	{
		std::stringstream ss;
		ss << "invalid sModule:" << sModule << ",sCmd:" << sCmd << std::endl;

		ASYNC_PIE_LOG(PIE_NOTICE, "{}", ss.str().c_str());
		return;
	}
	
	try
	{
		handler(*this, msg);
	}
	catch (std::exception& e)
	{
		std::stringstream ss;
		ss << "m_iIggId:" << m_iIggId << "|Unexpected exception: " << e.what();
		ASYNC_PIE_LOG(PIE_ERROR, "Exception:{}", ss.str().c_str());
	}
}

void MockRole::clearMsg()
{
	m_configCmd.clear();
	m_iCurIndex = 0;
}

void MockRole::pushMsg(::pubsub::TEST_CMD& msg)
{
	m_configCmd.push_back(msg);
}

bool MockRole::addHandler(const std::string& sModule, const std::string& sCmd, HandlerCb cb)
{
	auto findIte = m_cmdHandler.find(sModule);
	if (findIte != m_cmdHandler.end())
	{
		auto ite = findIte->second.find(sCmd);
		if (ite != findIte->second.end())
		{
			return false;
		}

		findIte->second[sCmd] = cb;
		return true;
	}

	std::map<std::string, HandlerCb> cmdMap;
	cmdMap[sCmd] = cb;
	m_cmdHandler[sModule] = cmdMap;

	return true;
}

MockRole::HandlerCb MockRole::findHandler(const std::string& sModule, const std::string& sCmd)
{
	auto findIte = m_cmdHandler.find(sModule);
	if (findIte == m_cmdHandler.end())
	{
		return nullptr;
	}

	auto ite = findIte->second.find(sCmd);
	if (ite == findIte->second.end())
	{
		return nullptr;
	}

	return ite->second;
}

bool MockRole::addResponseHandler(uint32_t opcodes, HandleResponseCB cb)
{
	auto findIte = m_responseHandler.find(opcodes);
	if (findIte != m_responseHandler.end())
	{
		return false;
	}

	m_responseHandler[opcodes] = cb;
	return true;
}

MockRole::HandleResponseCB MockRole::findResponseHandler(uint32_t opcodes)
{
	auto findIte = m_responseHandler.find(opcodes);
	if (findIte == m_responseHandler.end())
	{
		return nullptr;
	}

	return findIte->second;
}

void MockRole::clearResponseHandler()
{
	m_responseHandler.clear();
}

bool MockRole::registerDataHandler(uint32_t opcodes, HandleDataCB cb)
{
	auto findIte = m_dataHandler.find(opcodes);
	if (findIte != m_dataHandler.end())
	{
		return false;
	}

	m_dataHandler[opcodes] = cb;
	return true;
}

MockRole::HandleDataCB MockRole::findDataHandler(uint32_t opcodes)
{
	auto findIte = m_dataHandler.find(opcodes);
	if (findIte == m_dataHandler.end())
	{
		return nullptr;
	}

	return findIte->second;
}


void MockRole::setPauseProcess(bool flag)
{
	m_bPauseProcess = false;
}

std::map<std::tuple<uint32_t, uint32_t>, std::vector<uint64_t>>& MockRole::getReplyDelay()
{
	return m_replyDelay;
}

std::map<std::tuple<uint32_t, uint32_t>, std::tuple<uint64_t, uint64_t, uint64_t, uint64_t>>& MockRole::getMergeReplyDelay()
{
	return m_mergeReplyDelay;
}

bool MockRole::hasTimeout(uint64_t iCurMS)
{
	for (const auto& elems : m_pendingResponse)
	{
		if (iCurMS > elems.expired_at_ms)
		{
			return true;
		}
	}

	for (const auto& elems : m_pendingRPC)
	{
		if (iCurMS > elems.expired_at_ms)
		{
			return true;
		}
	}

	return false;
}

uint32_t MockRole::waitResponse(uint32_t response, uint32_t request, HandleResponseCB cb, uint32_t timeout)
{
	m_id++;

	auto iCurMs = Ctx::getCurMilliseconds();

	PendingResponse pendingObj;
	pendingObj.id = m_id;
	pendingObj.request_opcode = request;
	pendingObj.request_time = iCurMs;
	pendingObj.response_opcode = response;
	pendingObj.cb = cb;
	pendingObj.timeout = timeout;
	pendingObj.expired_at_ms = iCurMs + timeout;

	m_pendingResponse.push_back(pendingObj);

	return m_id;
}

void MockRole::waitRPC(uint32_t iRPCId, uint32_t request, HandleResponseCB cb, uint32_t timeout)
{
	auto iCurMs = Ctx::getCurMilliseconds();

	PendingRPC pendingObj;
	pendingObj.id = iRPCId;
	pendingObj.request_opcode = request;
	pendingObj.request_time = iCurMs;
	pendingObj.response_opcode = 0;
	pendingObj.cb = cb;
	pendingObj.timeout = timeout;
	pendingObj.expired_at_ms = iCurMs + timeout;

	m_pendingRPC.push_back(pendingObj);
}

std::optional<PendingRPC> MockRole::findWaitRPC(uint32_t iRPCId)
{
	for (auto& elems : m_pendingRPC)
	{
		if (elems.id == iRPCId)
		{
			return std::make_optional(elems);
		}
	}

	return std::nullopt;
}

bool MockRole::handleWaitRPC(MessageInfo info, const std::string& msg)
{
	auto findIte = findWaitRPC(info.iRPCRequestID);
	if (!findIte.has_value())
	{
		return false;
	}

	bool bHandled = false;
	findIte.value().response_opcode = info.iOpcode;
	if (findIte.value().cb)
	{
		findIte.value().cb(this, info, msg);
		bHandled = true;
	}

	auto iCurMs = Ctx::getCurMilliseconds();
	auto iDelay = iCurMs - findIte.value().request_time;
	std::tuple<uint32_t, uint32_t> key = { findIte.value().request_opcode, findIte.value().response_opcode };
	this->calculateCostTime(key, iDelay);


	removeWaitRPC(findIte.value().id);

	return bHandled;
}

bool MockRole::handleData(MessageInfo info, const std::string& msg)
{
	auto findIte = findDataHandler(info.iOpcode);
	if (findIte == nullptr)
	{
		return false;
	}

	findIte(this, info, msg);
	return true;
}

std::optional<PendingResponse> MockRole::findWaitResponse(uint32_t response)
{
	for (auto& elems : m_pendingResponse)
	{
		if (elems.response_opcode == response)
		{
			return std::make_optional(elems);
		}
	}

	return std::nullopt;
}

void MockRole::removeWaitResponse(uint32_t id)
{
	auto cmp = [id](const PendingResponse& elems) {
		if (elems.id == id)
		{
			return true;
		}

		return false;
	};

	m_pendingResponse.remove_if(cmp);
}

void MockRole::removeWaitRPC(uint32_t id)
{
	auto cmp = [id](const PendingRPC& elems) {
		if (elems.id == id)
		{
			return true;
		}

		return false;
	};

	m_pendingRPC.remove_if(cmp);
}

void MockRole::clearWait()
{
	m_pendingResponse.clear();
	m_pendingRPC.clear();
}

bool MockRole::handleResponse(MessageInfo info, const std::string& msg)
{
	auto serialNum = info.iRPCRequestID;
	auto opcodes = info.iOpcode;

	std::string sMsg = msg;
	do 
	{
		auto typeOpt = getPbNameByOpcode(opcodes);
		if (!typeOpt.has_value())
		{
			sMsg = "******unregisterPbMap******";
			break;
		}

		std::string sType = typeOpt.value();
		auto ptrMsg = apie::message::ProtobufFactory::createMessage(sType);
		if (ptrMsg == nullptr)
		{
			sMsg = "******createMessageError******";
			break;
		}

		std::shared_ptr<::google::protobuf::Message> newMsg(ptrMsg);
		bool bResult = newMsg->ParseFromString(msg);
		if (!bResult)
		{
			sMsg = "******ParseFromStringError******";
			break;
		}

		sMsg = newMsg->ShortDebugString();
	} while (false);

	std::stringstream ss;
	ss << "traffic/" << m_iIggId;
	std::string fileName = ss.str();

	ss.str("");

	auto [iType, iCmd] = SplitOpcode(opcodes);
	ss << "recv|iSessionId:" << info.iSessionId << "|iSeqId:" << info.iRPCRequestID << "|iOpcode:" << opcodes << ",iType:" << iType << ",iCmd:" << iCmd << "|data:" << sMsg;
	//ASYNC_PIE_LOG_CUSTOM(fileName.c_str(), PIE_CYCLE_DAY, PIE_DEBUG, "%s", ss.str().c_str());

	auto findIte = findResponseHandler(opcodes);
	if (findIte != nullptr)
	{
		findIte(this, info, msg);
		return true;
	}

	return false;
}

bool MockRole::handleWaitResponse(MessageInfo info, const std::string& msg)
{
	auto opcodes = info.iOpcode;
	auto findIte = findWaitResponse(opcodes);
	if (!findIte.has_value())
	{
		return false;
	}

	bool bHandled = false;
	if (findIte.value().cb)
	{
		findIte.value().cb(this, info, msg);
		bHandled = true;
	}

	auto iCurMs = Ctx::getCurMilliseconds();
	auto iDelay = iCurMs - findIte.value().request_time;
	std::tuple<uint32_t, uint32_t> key = { findIte.value().request_opcode, findIte.value().response_opcode };
	this->calculateCostTime(key, iDelay);

	removeWaitResponse(findIte.value().id);

	return bHandled;
}


std::shared_ptr<MockRole> MockRole::createMockRole(uint64_t iIggId)
{
	return std::make_shared<MockRole>(iIggId);
}

bool MockRole::registerPbOpcodeName(uint32_t iOpcode, const std::string& sName)
{
	auto findIte = s_pbReflect.find(iOpcode);
	if (findIte != s_pbReflect.end())
	{
		return false;
	}

	s_pbReflect[iOpcode] = sName;
	return true;
}

std::optional<std::string> MockRole::getPbNameByOpcode(uint32_t iOpcode)
{
	auto findIte = s_pbReflect.find(iOpcode);
	if (findIte == s_pbReflect.end())
	{
		return std::nullopt;
	}

	return findIte->second;
}

void MockRole::handleAccountLogin(MessageInfo info, const std::string& msg)
{
	this->setPauseProcess(true);

	auto serialNum = info.iRPCRequestID;
	auto opcodes = info.iOpcode;

	auto [iType, iCmd] = SplitOpcode(opcodes);
	std::stringstream ss;
	ss << "handleResponse|m_iIggId:" << m_iIggId << "|serialNum:" << serialNum << "|iOpcode:" << opcodes << ",iType:" << iType << ",iCmd:" << iCmd;


	::login_msg::AccountLoginResponse response;
	bool bResult = response.ParseFromString(msg);
	if (!bResult)
	{
		ASYNC_PIE_LOG(PIE_NOTICE, "{}", ss.str());

		APieGetModule<apie::TestServerMgr>()->removeMockRole(m_iIggId);
		return;
	}

	if (response.error_code() != pb::core::OK)
	{
		ss << "data|" << response.ShortDebugString();
		ASYNC_PIE_LOG(PIE_NOTICE, "{}", ss.str());

		APieGetModule<apie::TestServerMgr>()->removeMockRole(m_iIggId);
		return;
	}

	ss << "data|" << response.ShortDebugString();
	ASYNC_PIE_LOG(PIE_NOTICE, "{}", ss.str());

	APieGetModule<apie::TestServerMgr>()->removeSerialNum(this->m_clientProxy->getSerialNum());
	this->m_clientProxy->onActiveClose();


	std::string ip = response.ip();
	uint32_t port = response.port();
	uint16_t type = toUnderlyingType(apie::ProtocolType::PT_PB);
	uint32_t maskFlag = apie::CtxSingleton::get().getConfigs()->clients.socket_address.mask_flag;


	m_clientProxy = apie::ClientProxy::createClientProxy();
	std::weak_ptr<MockRole> ptrSelf = this->shared_from_this();
	auto connectCb = [ptrSelf, response](apie::ClientProxy* ptrClient, uint32_t iResult) mutable {
		if (iResult == 0)
		{
			auto ptrShared = ptrSelf.lock();
			if (ptrShared)
			{
				APieGetModule<apie::TestServerMgr>()->addSerialNumRole(ptrShared->m_clientProxy->getSerialNum(), ptrShared->m_iIggId);

				::login_msg::ClientLoginRequest request;
				request.set_user_id(ptrShared->m_iIggId);
				ptrShared->sendMsg(pb::core::OP_ClientLoginRequest, request);

				ptrShared->setPauseProcess(false);
			}
		}
		return true;
	};
	m_clientProxy->connect(ip, port, static_cast<apie::ProtocolType>(type), maskFlag, connectCb);
	m_clientProxy->addReconnectTimer(1000);
	m_target = CT_Gateway;
}

void MockRole::handleClientLogin(MessageInfo info, const std::string& msg)
{
	std::stringstream ss;
	ss << "handleResponse|m_iIggId:" << m_iIggId << "|serialNum:" << info.iRPCRequestID << "|iOpcode:" << info.iOpcode;

	::login_msg::ClientLoginResponse response;
	bool bResult = response.ParseFromString(msg);
	if (!bResult)
	{
		ASYNC_PIE_LOG(PIE_NOTICE, "recv:{}", ss.str());
		APieGetModule<apie::TestServerMgr>()->removeMockRole(m_iIggId);
		return;
	}

	ASYNC_PIE_LOG(PIE_NOTICE, "recv:{}|response:{}", ss.str(), response.ShortDebugString());
}

void MockRole::sendKeepAlive()
{
	auto iCurTime = time(nullptr);
	if (iCurTime < m_iNextKeepAliveTime || m_target != CT_Gateway)
	{
		return;
	}
	m_iNextKeepAliveTime = iCurTime + 120;
}

void MockRole::calculateCostTime(std::tuple<uint32_t, uint32_t> key, uint32_t iDelay)
{
	auto ite = m_replyDelay.find(key);
	if (ite == m_replyDelay.end())
	{
		std::vector<uint64_t> delayVec;
		delayVec.push_back(iDelay);

		m_replyDelay[key] = delayVec;
	}
	else
	{
		if (ite->second.size() > 100)
		{
			uint64_t iMin = 0;
			uint64_t iMax = 0;
			//uint64_t iAverage = 0;
			uint64_t iTotal = 0;
			uint64_t iCount = 0;
			for (auto& items : ite->second)
			{
				if (iMin == 0)
				{
					iMin = items;
				}

				if (iMax == 0)
				{
					iMax = items;
				}

				if (iMin > items)
				{
					iMin = items;
				}

				if (iMax < items)
				{
					iMax = items;
				}

				iCount++;

				iTotal += items;
			}

			auto findIte = m_mergeReplyDelay.find(key);
			if (findIte == m_mergeReplyDelay.end())
			{
				std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> mergeElem = { iMin, iMax, iCount, iTotal };
				m_mergeReplyDelay[key] = mergeElem;
			}
			else
			{
				auto prevElem = m_mergeReplyDelay[key];

				uint64_t iCurMin = std::get<0>(prevElem);
				if (iMin < iCurMin)
				{
					iCurMin = iMin;
				}

				uint64_t iCurMax = std::get<1>(prevElem);
				if (iMax > iCurMax)
				{
					iCurMax = iMax;
				}

				uint64_t iCurCount = std::get<2>(prevElem) + iCount;
				uint64_t iCurTotal = std::get<3>(prevElem) + iTotal;

				std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> mergeElem = { iCurMin, iCurMax, iCurCount, iCurTotal };
				m_mergeReplyDelay[key] = mergeElem;
			}

			ite->second.clear();
		}

		ite->second.push_back(iDelay);
	}
}

void MockRole::setSSeqId(uint32_t iId)
{
	m_iSSeqId = iId;
}

uint64_t MockRole::sendMsg(uint32_t iOpcode, const ::google::protobuf::Message& msg)
{
	m_clientProxy->sendMsg(iOpcode, msg);

	auto iSessionId = m_clientProxy->getSerialNum();
	auto iSeqId = m_clientProxy->getSequenceNumber();

	std::stringstream ss;
	ss << "traffic/" << m_iIggId;
	std::string fileName = ss.str();

	ss.str("");

	auto [iType, iCmd] = SplitOpcode(iOpcode);
	ss << "send|iSessionId:" << iSessionId << "|iSeqId:" << iSeqId << "|iOpcode:" << iOpcode  << ",iType:" << iType << ",iCmd:" << iCmd << "|data:" << msg.ShortDebugString();
	
	//ASYNC_PIE_LOG_CUSTOM(fileName.c_str(), PIE_CYCLE_DAY, PIE_DEBUG, "%s", ss.str().c_str());

	return iSeqId;
}

}

