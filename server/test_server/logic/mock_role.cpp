#include "logic/mock_role.h"

#include <functional>

#include "logic/test_server.h"

#include "../../common/opcodes.h"

namespace apie {

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

	this->addHandler("login", std::bind(&MockRole::handleLogin, this, std::placeholders::_1));
	this->addHandler("echo", std::bind(&MockRole::handleEcho, this, std::placeholders::_1));
	this->addHandler("logout", std::bind(&MockRole::handleLogout, this, std::placeholders::_1));


	this->addResponseHandler(MergeOpcode(::apie::_MSG_GAMESERVER_LOGINRESP, 0), &MockRole::handle_MSG_GAMESERVER_LOGINRESP);
	this->addResponseHandler(MergeOpcode(::apie::_MSG_USER_INFO, pb::userinfo::E_UserFlag_New), &MockRole::handle_MSG_USER_INFO_E_UserFlag_New);
	

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

		if (!m_waitResponse.empty())
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

void MockRole::handleMsg(::pubsub::LOGIC_CMD& msg)
{
	auto sCmd = msg.cmd();
	auto handler = this->findHandler(sCmd);
	if (handler == nullptr)
	{
		std::stringstream ss;
		ss << "invalid cmd:" << sCmd << std::endl;
		PIE_LOG("MockRole/handleMsg", PIE_CYCLE_HOUR, PIE_ERROR, "%s", ss.str().c_str());
		return;
	}
	
	try
	{
		handler(msg);
	}
	catch (std::exception& e)
	{
		std::stringstream ss;
		ss << "m_iIggId:" << m_iIggId << "|Unexpected exception: " << e.what();
		PIE_LOG("Exception/Exception", PIE_CYCLE_HOUR, PIE_ERROR, "%s: %s", "Exception", ss.str().c_str());
	}
}

void MockRole::clearMsg()
{
	m_configCmd.clear();
	m_iCurIndex = 0;
}

void MockRole::pushMsg(::pubsub::LOGIC_CMD& msg)
{
	m_configCmd.push_back(msg);
}

bool MockRole::addHandler(const std::string& name, HandlerCb cb)
{
	auto findIte = m_cmdHandler.find(name);
	if (findIte != m_cmdHandler.end())
	{
		return false;
	}

	m_cmdHandler[name] = cb;
	return true;
}

MockRole::HandlerCb MockRole::findHandler(const std::string& name)
{
	auto findIte = m_cmdHandler.find(name);
	if (findIte == m_cmdHandler.end())
	{
		return nullptr;
	}

	return findIte->second;
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

void MockRole::setPauseProcess(bool flag)
{
	m_bPauseProcess = false;
}

void MockRole::addWaitResponse(uint32_t iOpcode, uint32_t iNeedCheck)
{
	m_waitResponse[iOpcode] = iNeedCheck;
}

void MockRole::removeWaitResponse(uint32_t iOpcode)
{
	m_waitResponse.erase(iOpcode);
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

	for (const auto& elems : m_pendingNotify)
	{
		if (iCurMS > elems.expired_at_ms)
		{
			return true;
		}
	}

	return false;
}

uint32_t MockRole::addPendingResponse(uint32_t response, uint32_t request, HandleResponseCB cb, uint32_t timeout)
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

std::optional<PendingResponse> MockRole::findPendingResponse(uint32_t response)
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

void MockRole::removePendingResponseById(uint32_t id)
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

void MockRole::clearPendingResponse()
{
	m_pendingResponse.clear();
}


uint32_t MockRole::addPendingNotify(uint32_t response, HandleResponseCB cb, uint32_t timeout)
{
	m_id++;

	auto iCurMs = Ctx::getCurMilliseconds();

	PendingNotify pendingObj;
	pendingObj.id = m_id;
	pendingObj.response_opcode = response;
	pendingObj.cb = cb;
	pendingObj.timeout = timeout;
	pendingObj.expired_at_ms = iCurMs + timeout;

	m_pendingNotify.push_back(pendingObj);

	return m_id;
}

std::optional<PendingNotify> MockRole::findPendingNotify(uint32_t response)
{
	for (auto& elems : m_pendingNotify)
	{
		if (elems.response_opcode == response)
		{
			return std::make_optional(elems);
		}
	}

	return std::nullopt;
}

void MockRole::removePendingNotifyById(uint32_t id)
{
	auto cmp = [id](const PendingNotify& elems) {
		if (elems.id == id)
		{
			return true;
		}

		return false;
	};

	m_pendingNotify.remove_if(cmp);
}

void MockRole::clearPendingNotify()
{
	m_pendingNotify.clear();
}

void MockRole::handlePendingNotify(MessageInfo info, const std::string& msg)
{
	auto findIte = findPendingNotify(info.iOpcode);
	if (!findIte.has_value())
	{
		return;
	}

	if (findIte.value().cb)
	{
		findIte.value().cb(this, info, msg);
	}
	
	removePendingNotifyById(findIte.value().id);
}


void MockRole::handleResponse(MessageInfo info, const std::string& msg)
{
	auto serialNum = info.iSeqNum;
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
	ss << "recv|iSerialNum:" << serialNum << "|iOpcode:" << opcodes << ",iType:" << iType << ",iCmd:" << iCmd << "|data:" << sMsg;
	ASYNC_PIE_LOG_CUSTOM(fileName.c_str(), PIE_CYCLE_DAY, PIE_DEBUG, "%s", ss.str().c_str());

	this->removeWaitResponse(opcodes);

	auto findIte = findResponseHandler(opcodes);
	if (findIte != nullptr)
	{
		findIte(this, info, msg);

		//std::cout << ss.str() << std::endl;
		return;
	}

	//std::cout << ss.str() << std::endl;
}

void MockRole::handlePendingResponse(MessageInfo info, const std::string& msg)
{
	auto opcodes = info.iOpcode;
	auto findIte = findPendingResponse(opcodes);
	if (!findIte.has_value())
	{
		return;
	}

	auto iCurMs = Ctx::getCurMilliseconds();
	auto iDelay = iCurMs - findIte.value().request_time;

	std::tuple<uint32_t, uint32_t> key = { findIte.value().request_opcode, findIte.value().response_opcode};
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

	if (findIte.value().cb)
	{
		findIte.value().cb(this, info, msg);
	}

	removePendingResponseById(findIte.value().id);
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

void MockRole::handleLogin(::pubsub::LOGIC_CMD& msg)
{
	pb::login::LoginC2LS request;
	request.set_game_id(1);
	request.set_user_id(m_iIggId);
	request.set_version(1);
	this->sendMsg(MergeOpcode(::apie::_MSG_CLIENT_LOGINTOL, 0), request);

	this->addPendingResponse(MergeOpcode(::apie::_MSG_GAMESERVER_LOGINRESP, 0), MergeOpcode(::apie::_MSG_CLIENT_LOGINTOL, 0));
}

void MockRole::handleEcho(::pubsub::LOGIC_CMD& msg)
{
	::login_msg::MSG_REQUEST_ECHO request;
	request.set_value1(std::stoi(msg.params()[0]));
	request.set_value2(msg.params()[1]);

	this->sendMsg(::apie::OP_MSG_REQUEST_ECHO, request);
	this->addPendingResponse(OP_MSG_RESPONSE_ECHO, OP_MSG_REQUEST_ECHO);
}

void MockRole::handleLogout(::pubsub::LOGIC_CMD& msg)
{
	APieGetModule<apie::TestServerMgr>()->removeMockRole(m_iIggId);

	uint64_t iSerialNum = 0;
	if (m_clientProxy)
	{
		iSerialNum = m_clientProxy->getSerialNum();
	}

	MessageInfo info;
	info.iSeqNum = iSerialNum;
	info.iOpcode = 0;
	this->handlePendingNotify(info, "active close");
}

void MockRole::handle_MSG_GAMESERVER_LOGINRESP(MessageInfo info, const std::string& msg)
{
	this->setPauseProcess(true);

	auto serialNum = info.iSeqNum;
	auto opcodes = info.iOpcode;

	auto [iType, iCmd] = SplitOpcode(opcodes);
	std::stringstream ss;
	ss << "handleResponse|m_iIggId:" << m_iIggId << "|serialNum:" << serialNum << "|iOpcode:" << opcodes << ",iType:" << iType << ",iCmd:" << iCmd;


	pb::login::LoginLS_Resp response;
	bool bResult = response.ParseFromString(msg);
	if (!bResult)
	{
		ASYNC_PIE_LOG("handleResponse/recv", PIE_CYCLE_HOUR, PIE_NOTICE, "%s", ss.str().c_str());

		APieGetModule<apie::TestServerMgr>()->removeMockRole(m_iIggId);
		return;
	}

	if (response.result() != pb::login::E_Login_Result::Succ)
	{
		ss << "data|" << response.ShortDebugString();
		ASYNC_PIE_LOG("handleResponse/recv", PIE_CYCLE_HOUR, PIE_NOTICE, "%s", ss.str().c_str());

		APieGetModule<apie::TestServerMgr>()->removeMockRole(m_iIggId);
		return;
	}

	ss << "data|" << response.ShortDebugString();
	ASYNC_PIE_LOG("handleResponse/recv", PIE_CYCLE_HOUR, PIE_NOTICE, "%s", ss.str().c_str());

	APieGetModule<apie::TestServerMgr>()->removeSerialNum(this->m_clientProxy->getSerialNum());
	this->m_clientProxy->onActiveClose();


	std::string ip = response.ip();
	uint32_t port = response.port();
	uint16_t type = toUnderlyingType(apie::ProtocolType::PT_PBMsgUser);
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


				pb::login::LoginC2GS request;
				request.set_game_id(1);
				request.set_check_out_text(response.check_out_text());
				request.set_user_id(ptrShared->m_iIggId);
				ptrShared->sendMsg(MergeOpcode(_MSG_CLIENT_LOGINTOG, 0), request);

				ptrShared->setPauseProcess(false);
				ptrShared->addWaitResponse(::apie::OP_MSG_RESPONSE_CLIENT_LOGIN, 1);
			}
		}
		return true;
	};
	m_clientProxy->connect(ip, port, static_cast<apie::ProtocolType>(type), maskFlag, connectCb);
	m_clientProxy->addReconnectTimer(1000);
	m_target = CT_Gateway;
}

void MockRole::handle_MSG_USER_INFO_E_UserFlag_New(MessageInfo info, const std::string& msg)
{
	auto [iType, iCmd] = SplitOpcode(info.iOpcode);
	std::stringstream ss;
	ss << "handleResponse|m_iIggId:" << m_iIggId << "|serialNum:" << info.iSeqNum << "|iOpcode:" << info.iOpcode << ",iType:" << iType << ",iCmd:" << iCmd;

	pb::userinfo::NewUser response;
	bool bResult = response.ParseFromString(msg);
	if (!bResult)
	{
		ASYNC_PIE_LOG("handleResponse/recv", PIE_CYCLE_HOUR, PIE_NOTICE, "%s", ss.str().c_str());

		APieGetModule<apie::TestServerMgr>()->removeMockRole(m_iIggId);
		return;
	}

	if (response.flag() == pb::userinfo::E_UserFlag_New)
	{
		pb::map::Choose_Country request;
		request.set_country_id(2);
		this->sendMsg(MergeOpcode(::apie::_MSG_MAP_USER_CMD, pb::map::ChooseCountry), request);
		this->sendMsg(MergeOpcode(::apie::_MSG_MAP_USER_CMD, pb::map::CmdEnterMap), request);
	}
}



void MockRole::sendMsg(uint32_t iOpcode, const ::google::protobuf::Message& msg)
{
	m_clientProxy->sendMsg(iOpcode, msg);

	auto iSerialNum = m_clientProxy->getSerialNum();

	std::stringstream ss;
	ss << "traffic/" << m_iIggId;
	std::string fileName = ss.str();

	ss.str("");

	auto [iType, iCmd] = SplitOpcode(iOpcode);
	ss << "send|iSerialNum:" << iSerialNum << "|iOpcode:" << iOpcode  << ",iType:" << iType << ",iCmd:" << iCmd << "|data:" << msg.ShortDebugString();
	ASYNC_PIE_LOG_CUSTOM(fileName.c_str(), PIE_CYCLE_DAY, PIE_DEBUG, "%s", ss.str().c_str());
}

}

