#include "logic/test_server.h"

#include "json/json.h"

#include "../../common/opcodes.h"

#include "logic/test_runner.h"
#include "module_handler/test_server_module.h"

namespace apie {

std::string TestServerMgr::moduleName()
{
	return "TestServerMgr";
}

uint32_t TestServerMgr::modulePrecedence()
{
	return 1;
}

TestServerMgr::TestServerMgr(std::string name, module_loader::ModuleLoaderBase* prtLoader)
	: m_name(name),
	m_prtLoader(prtLoader)
{

}

apie::status::Status TestServerMgr::init()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_Test_Client });
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}

	TestServerModule::init();

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestServerMgr::start()
{
	std::string ip = apie::CtxSingleton::get().getConfigs()->clients.socket_address.address;
	uint16_t port = apie::CtxSingleton::get().getConfigs()->clients.socket_address.port_value;
	uint16_t type = apie::CtxSingleton::get().getConfigs()->clients.socket_address.type;
	uint32_t maskFlag = apie::CtxSingleton::get().getConfigs()->clients.socket_address.mask_flag;

	m_ptrClientProxy = apie::ClientProxy::createClientProxy();
	auto connectCb = [this](apie::ClientProxy* ptrClient, uint32_t iResult) {
		if (iResult == 0)
		{
			this->setHookReady(apie::hook::HookPoint::HP_Start);
		}
		return true;
	};
	m_ptrClientProxy->connect(ip, port, static_cast<apie::ProtocolType>(type), maskFlag, connectCb);
	m_ptrClientProxy->addReconnectTimer(60000);

	return { apie::status::StatusCode::OK_ASYNC, "" };
}

apie::status::Status TestServerMgr::ready()
{
	TestServerModule::ready();

	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ServerStatus", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status TestServerMgr::exit()
{
	return { apie::status::StatusCode::OK, "" };
}

void TestServerMgr::setHookReady(hook::HookPoint point)
{
	m_prtLoader->setHookReady(point);
}

void TestServerMgr::addMockRole(std::shared_ptr<MockRole> ptrMockRole)
{
	auto iRoleId = ptrMockRole->getRoleId();
	m_mockRole[iRoleId] = ptrMockRole;
}

std::shared_ptr<MockRole> TestServerMgr::findMockRole(uint64_t iRoleId)
{
	auto findIte = m_mockRole.find(iRoleId);
	if (findIte == m_mockRole.end())
	{
		return nullptr;
	}

	return findIte->second;
}

void TestServerMgr::removeMockRole(uint64_t iRoleId)
{
	m_mockRole.erase(iRoleId);
}

void TestServerMgr::addSerialNumRole(uint64_t iSerialNum, uint64_t iRoleId)
{
	m_serialNumRole[iSerialNum] = iRoleId;
}

std::optional<uint64_t> TestServerMgr::findRoleIdBySerialNum(uint64_t iSerialNum)
{
	auto findIte = m_serialNumRole.find(iSerialNum);
	if (findIte == m_serialNumRole.end())
	{
		return std::nullopt;
	}

	return findIte->second;
}

void TestServerMgr::removeSerialNum(uint64_t iSerialNum)
{
	m_serialNumRole.erase(iSerialNum);
}


}

