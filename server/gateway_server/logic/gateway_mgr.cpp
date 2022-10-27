#include "logic/gateway_mgr.h"

#include "../../common/dao/model_user.h"
#include "../../common/dao/model_role_extra.h"
#include "../../common/dao/model_account.h"
#include "../../common/dao/model_account_name.h"
#include "../../common/opcodes.h"

#include "logic/gateway_role.h"

#include "module_handler/gateway_mgr_module.h"


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
	std::shared_ptr<std::map<DeclarativeBase::DBType, bool>> ptrReady = std::make_shared<std::map<DeclarativeBase::DBType, bool>>();
	ptrReady->insert({ DeclarativeBase::DBType::DBT_Role, false });
	ptrReady->insert({ DeclarativeBase::DBType::DBT_Account, false });

	{
		// 加载:数据表结构
		auto dbType = DeclarativeBase::DBType::DBT_Role;
		auto ptrReadyCb = [this, ptrReady](bool bResul, std::string sInfo, uint64_t iCallCount) mutable {
			if (!bResul)
			{
				std::stringstream ss;
				ss << "CallMysqlDescTable|bResul:" << bResul << ",sInfo:" << sInfo << ",iCallCount:" << iCallCount;

				PANIC_ABORT(ss.str().c_str());
			}

			(*ptrReady)[DeclarativeBase::DBType::DBT_Role] = true;
			for (const auto& elems : *ptrReady)
			{
				if (elems.second == false)
				{
					return;
				}
			}
			this->setHookReady(hook::HookPoint::HP_Start);
		};

		::rpc_msg::CHANNEL server;
		server.set_realm(apie::Ctx::getThisChannel().realm());
		server.set_type(::common::EPT_DB_ROLE_Proxy);
		server.set_id(1);

		std::map<std::string, DAOFactory::TCreateMethod> loadTables;
		loadTables.insert(std::make_pair(ModelUser::getFactoryName(), ModelUser::createMethod));
		loadTables.insert(std::make_pair(ModelRoleExtra::getFactoryName(), ModelRoleExtra::createMethod));

		bool bResult = RegisterRequiredTable(server, dbType, loadTables, ptrReadyCb);
		if (!bResult)
		{
			return { apie::status::StatusCode::HOOK_ERROR, "HR_Error" };
		}
	}

	{
		auto dbType = DeclarativeBase::DBType::DBT_Account;
		auto ptrReadyCb = [this, ptrReady](bool bResul, std::string sInfo, uint64_t iCallCount) mutable {
			if (!bResul)
			{
				std::stringstream ss;
				ss << "CallMysqlDescTable|bResul:" << bResul << ",sInfo:" << sInfo << ",iCallCount:" << iCallCount;

				PANIC_ABORT(ss.str().c_str());
			}

			(*ptrReady)[DeclarativeBase::DBType::DBT_Account] = true;

			for (const auto& elems : *ptrReady)
			{
				if (elems.second == false)
				{
					return;
				}
			}
			this->setHookReady(apie::hook::HookPoint::HP_Start);
		};

		::rpc_msg::CHANNEL server;
		server.set_realm(apie::Ctx::getThisChannel().realm());
		server.set_type(::common::EPT_DB_ACCOUNT_Proxy);
		server.set_id(1);

		std::map<std::string, DAOFactory::TCreateMethod> loadTables;
		loadTables.insert(std::make_pair(ModelAccount::getFactoryName(), ModelAccount::createMethod));
		loadTables.insert(std::make_pair(ModelAccountName::getFactoryName(), ModelAccountName::createMethod));

		bool bResult = RegisterRequiredTable(server, dbType, loadTables, ptrReadyCb);
		if (!bResult)
		{
			return { apie::status::StatusCode::HOOK_ERROR, "HR_Error" };
		}
	}

	return { apie::status::StatusCode::OK_ASYNC, "" };

}

apie::status::Status GatewayMgr::ready()
{
	GatewayMgrModule::ready();

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

