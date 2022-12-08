#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>
#include <memory>
#include <functional>
#include <optional>

#include "apie/mysql_driver/mysql_orm.h"

#include "apie/singleton/threadsafe_singleton.h"
#include "apie/proto/init.h"
#include "apie/configs/configs.h"
#include "apie/api/hook.h"
#include "apie/network/logger.h"
#include "apie/common/dao_macros.h"



namespace apie {

	class DAOFactory
	{
	public:
		using TCreateMethod = std::function<std::shared_ptr<DeclarativeBase>()>;

	public:
		bool registerFactory(const std::string name, TCreateMethod funcCreate);
		std::shared_ptr<DeclarativeBase> create(const std::string& name);
		std::map<std::string, TCreateMethod>& getMethods();

		bool addTable(const std::string& name, MysqlTable& table);
		std::optional<MysqlTable> getTable(const std::string& name);

	private:
		std::map<std::string, TCreateMethod> m_methods;
		std::map<std::string, MysqlTable> m_tables;
	};

	class DAOFactoryType
	{
	public:
		bool registerRequiredTable(DeclarativeBase::DBType type, const std::string& name, DAOFactory::TCreateMethod funcCreate);
		std::optional<std::map<std::string, DAOFactory::TCreateMethod>> getRequiredTable(DeclarativeBase::DBType type);

		std::shared_ptr<DeclarativeBase> createDAO(DeclarativeBase::DBType type, const std::string& name);

		bool addLoadedTable(DeclarativeBase::DBType type, const std::string& name, MysqlTable& table);
		DAOFactory* getDAOFactory(DeclarativeBase::DBType type);

	private:
		DAOFactory account;
		DAOFactory role;
		DAOFactory config_db;
	};

	typedef ThreadSafeSingleton<DAOFactoryType> DAOFactoryTypeSingleton;


	using CallMysqlDescTableCB = std::function<void(bool bResul, std::string sInfo, uint64_t iCallCount)>;


	// ���ر�ṹ���Ѽ����б�RPC����ʧ�ܻ�һֱ���ԣ�ֱ�����óɹ������óɹ���Żᴥ���ص�
	bool CallMysqlDescTable(::rpc_msg::CHANNEL server, DeclarativeBase::DBType dbType, std::vector<std::string> tables, CallMysqlDescTableCB cb, uint64_t iCallCount = 0);


	bool RegisterRequiredTable(const ::rpc_msg::CHANNEL& server, DeclarativeBase::DBType type, const std::map<std::string, DAOFactory::TCreateMethod> &loadTables, CallMysqlDescTableCB cb);

	template<typename T>
	apie::status::Status DoBindTables(T* ptrMgr, uint32_t iRealm, APieConfig_BindTables tables)
	{
		std::shared_ptr<std::map<DeclarativeBase::DBType, bool>> ptrReady = std::make_shared<std::map<DeclarativeBase::DBType, bool>>();
		for (const auto& elems : tables.database)
		{
			ptrReady->insert({ (DeclarativeBase::DBType)elems.type, false });
		}

		for (const auto& elems : tables.database)
		{
			// ����:���ݱ�ṹ
			auto dbType = (DeclarativeBase::DBType)elems.type;
			auto ptrReadyCb = [ptrMgr, ptrReady, dbType](bool bResul, std::string sInfo, uint64_t iCallCount) mutable {
				if (!bResul)
				{
					std::stringstream ss;
					ss << "CallMysqlDescTable|bResul:" << bResul << ",sInfo:" << sInfo << ",iCallCount:" << iCallCount;

					PANIC_ABORT(ss.str().c_str());
				}

				(*ptrReady)[dbType] = true;
				for (const auto& elems : *ptrReady)
				{
					if (elems.second == false)
					{
						return;
					}
				}
				ptrMgr->setHookReady(hook::HookPoint::HP_Start);
			};

			auto iServerProxyType = ::common::EPT_None;
			switch (dbType)
			{
			case DeclarativeBase::DBType::DBT_Account:
			{
				iServerProxyType = ::common::EPT_DB_ACCOUNT_Proxy;
				break;
			}
			case DeclarativeBase::DBType::DBT_Role:
			{
				iServerProxyType = ::common::EPT_DB_ROLE_Proxy;
				break;
			}
			default:
				break;
			}

			::rpc_msg::CHANNEL server;
			server.set_realm(iRealm);
			server.set_type(iServerProxyType);
			server.set_id(elems.server_id);

			std::map<std::string, DAOFactory::TCreateMethod> loadTables;
			for (const auto& name : elems.table_name)
			{
				auto methodOpt = RegisterMetaTable::GetCreateMethond(dbType, name);
				if (!methodOpt.has_value())
				{
					return { apie::status::StatusCode::HOOK_ERROR, "HR_Error" };
				}

				loadTables.insert(std::make_pair(name, methodOpt.value()));
			}

			bool bResult = RegisterRequiredTable(server, dbType, loadTables, ptrReadyCb);
			if (!bResult)
			{
				return { apie::status::StatusCode::HOOK_ERROR, "HR_Error" };
			}
		}

		return { apie::status::StatusCode::OK_ASYNC, "" };

	}
}
