#include "logic/init_service/gateway_mgr.h"

#include "../../../common/opcodes.h"

#include "logic/init_service/gateway_role.h"
#include "logic/init_service/gateway_mgr_module.h"


namespace apie {

std::string GatewayMgr::moduleName()
{
	return "GatewayMgr";
}

uint32_t GatewayMgr::modulePrecedence()
{
	return 1;
}

GatewayMgr::GatewayMgr(std::string name, module_loader::ModuleLoaderBase* prtLoader)
	: m_name(name),
	m_prtLoader(prtLoader)
{

}

apie::status::Status GatewayMgr::init()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_Gateway_Server });
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}

	std::string pubKey = apie::CtxSingleton::get().getConfigs()->certificate.public_key;
	std::string privateKey = apie::CtxSingleton::get().getConfigs()->certificate.private_key;

	std::string errInfo;
	bResult = apie::crypto::RSAUtilitySingleton::get().init(pubKey, privateKey, errInfo);
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, errInfo };
	}
	
	GatewayMgrModule::init();


	return { apie::status::StatusCode::OK, "" };
}


apie::status::Status GatewayMgr::start()
{
	return DoBindTables<GatewayMgr>(this, hook::HookPoint::HP_Start, apie::Ctx::getThisChannel().realm(), apie::CtxSingleton::get().getConfigs()->bind_tables);
}

CoTaskVoid TestCoRPC1()
{
	int i = 0;
	i++;

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ACCOUNT_Proxy);
	server.set_id(1);

	::mysql_proxy_msg::MysqlDescribeRequest args;
	auto ptrAdd = args.add_names();
	*ptrAdd = "account";

	auto ptrAwait = MakeCoAwaitable<::mysql_proxy_msg::MysqlDescribeRequest, ::mysql_proxy_msg::MysqlDescribeResponse>(server, rpc_msg::RPC_MysqlDescTable, args);
	auto response = co_await *ptrAwait;
	if (!response.ok())
	{
		co_return;
	}

	auto valueObj = response.value();

	i++;

	co_return;
}

CoTaskVoid TestCoRPC2()
{
	int i = 0;
	i++;

	TestCoRPC1();

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ACCOUNT_Proxy);
	server.set_id(1);

	::mysql_proxy_msg::MysqlDescribeRequest args;
	auto ptrAdd = args.add_names();
	*ptrAdd = "account111";

	auto ptrAwait = MakeCoAwaitable<::mysql_proxy_msg::MysqlDescribeRequest, ::mysql_proxy_msg::MysqlDescribeResponse>(server, rpc_msg::RPC_MysqlDescTable, args);
	auto response = co_await *ptrAwait;
	if (!response.ok())
	{
		co_return;
	}

	auto valueObj = response.value();

	i++;

	co_return;
}

apie::status::Status GatewayMgr::ready()
{
	GatewayMgrModule::ready();

	TestCoRPC2();

	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ServerStatus", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status GatewayMgr::exit()
{
	return { apie::status::StatusCode::OK, "" };
}

void GatewayMgr::setHookReady(hook::HookPoint point)
{
	if (m_prtLoader->getHookReady(point))
	{
		return;
	}

	m_prtLoader->setHookReady(point);
}

std::shared_ptr<GatewayRole> GatewayMgr::findGatewayRoleById(uint64_t iRoleId)
{
	auto findIte = m_roleIdMapSerialNum.find(iRoleId);
	if (findIte == m_roleIdMapSerialNum.end())
	{
		return nullptr;
	}

	return findGatewayRoleBySerialNum(findIte->second);
}

std::shared_ptr<GatewayRole> GatewayMgr::findGatewayRoleBySerialNum(uint64_t iSerialNum)
{
	auto findIte = m_serialNumMap.find(iSerialNum);
	if (findIte == m_serialNumMap.end())
	{
		return nullptr;
	}

	return findIte->second;
}

std::optional<uint64_t> GatewayMgr::findRoleIdBySerialNum(uint64_t iSerialNum)
{
	auto findIte = m_serialNumMap.find(iSerialNum);
	if (findIte == m_serialNumMap.end())
	{
		return std::nullopt;
	}

	return findIte->second->getRoleId();
}

void GatewayMgr::addPendingRole(const PendingLoginRole &role)
{
	m_pendingRole[role.role_id] = role;
}

std::optional<PendingLoginRole> GatewayMgr::getPendingRole(uint64_t iRoleId)
{
	auto findIte = m_pendingRole.find(iRoleId);
	if (findIte == m_pendingRole.end())
	{
		return std::nullopt;
	}

	return findIte->second;
}

void GatewayMgr::removePendingRole(uint64_t iRoleId)
{
	m_pendingRole.erase(iRoleId);
}

bool GatewayMgr::addGatewayRole(std::shared_ptr<GatewayRole> ptrGatewayRole)
{
	if (ptrGatewayRole == nullptr)
	{
		return false;
	}

	auto iRoleId = ptrGatewayRole->getRoleId();
	auto iSerialNum = ptrGatewayRole->getSerailNum();

	auto ptrConnection = event_ns::DispatcherImpl::getConnection(iSerialNum);
	if (ptrConnection != nullptr)
	{
		ptrGatewayRole->setMaskFlag(ptrConnection->getMaskFlag());
	}

	auto ptrExist = findGatewayRoleById(iRoleId);
	if (ptrExist != nullptr)
	{
		return false;
	}

	m_serialNumMap[iSerialNum] = ptrGatewayRole;
	m_roleIdMapSerialNum[iRoleId] = iSerialNum;

	return true;
}

bool GatewayMgr::removeGateWayRole(uint64_t iRoleId)
{
	auto findIte = m_roleIdMapSerialNum.find(iRoleId);
	if (findIte == m_roleIdMapSerialNum.end())
	{
		return false;
	}

	m_serialNumMap.erase(findIte->second);
	m_roleIdMapSerialNum.erase(findIte);
	return true;
}

}

