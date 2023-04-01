#include "logic/dbproxy_mgr.h"

#include "logic/table_cache_mgr.h"

#include "module_handler/dbproxy_mgr_module.h"

#include "../../common/dao/init.h"

namespace apie {

std::string DBProxyMgr::moduleName()
{
	return "DBProxyMgr";
}

uint32_t DBProxyMgr::modulePrecedence()
{
	return 1;
}

DBProxyMgr::DBProxyMgr(std::string name, module_loader::ModuleLoaderBase* prtLoader)
	: m_name(name),
	m_prtLoader(prtLoader)
{

}

apie::status::Status DBProxyMgr::init()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_DbAccount_Proxy, ::common::EPT_DbRole_Proxy });
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}

	DBProxyMgrModule::init();

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status DBProxyMgr::start()
{
	DeclarativeBase::DBType dbType = DeclarativeBase::DBType::DBT_None;

	auto type = apie::CtxSingleton::get().getServerType();
	switch (type)
	{
	case ::common::EPT_DbAccount_Proxy:
	{
		dbType = DeclarativeBase::DBType::DBT_Account;
		break;
	}
	case ::common::EPT_DbRole_Proxy:
	{
		dbType = DeclarativeBase::DBType::DBT_Role;
		break;
	}
	default:
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}
	}

	auto registerOpt = RegisterMetaTable::GetRegisterMetaTableByType(dbType);
	if (!registerOpt.has_value())
	{
		return { apie::status::StatusCode::OK, "" };;
	}

	for (const auto& elems : registerOpt.value())
	{
		DAOFactoryTypeSingleton::get().registerRequiredTable(dbType, elems.first, elems.second);
	}

	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "null ptrDispatched" };
	}

	auto requiredTableOpt = DAOFactoryTypeSingleton::get().getRequiredTable(dbType);
	if (!requiredTableOpt.has_value())
	{
		return { apie::status::StatusCode::OK, "" };
	}

	std::vector<std::string> tables;
	for (const auto& items : requiredTableOpt.value())
	{
		tables.push_back(items.first);
	}

	for (const auto& tableName : tables)
	{
		MysqlTable table;
		bool bSQL = ptrDispatched->getMySQLConnector().describeTable(tableName, table);
		if (bSQL)
		{
			TableCacheMgrSingleton::get().addTable(table);

			auto ptrDaoBase = DAOFactoryTypeSingleton::get().createDAO(dbType, tableName);
			if (ptrDaoBase == nullptr)
			{
				std::stringstream ss;
				ss << "tableName:" << tableName << " not declare";

				return { apie::status::StatusCode::HOOK_ERROR, ss.str() };
			}

			ptrDaoBase->initMetaData(table);
			bool bResult = ptrDaoBase->isValid();
			if (!bResult)
			{
				std::stringstream ss;
				ss << "tableName:" << tableName << " isValid false";

				return { apie::status::StatusCode::HOOK_ERROR, ss.str() };
			}
		}
		else
		{
			return { apie::status::StatusCode::HOOK_ERROR, ptrDispatched->getMySQLConnector().getError() };
		}
	}

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status DBProxyMgr::ready()
{
	DBProxyMgrModule::ready();

	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG(PIE_NOTICE, "ServerStatus|{}", ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status DBProxyMgr::exit()
{
	return { apie::status::StatusCode::OK, "" };
}

void DBProxyMgr::setHookReady(hook::HookPoint point)
{
	if (m_prtLoader->getHookReady(point))
	{
		return;
	}

	m_prtLoader->setHookReady(point);
}

}

