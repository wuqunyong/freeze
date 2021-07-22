#include "apie/mysql_driver/dao_factory.h"

#include "apie/network/command.h"
#include "apie/network/ctx.h"
#include "apie/network/logger.h"
#include "apie/rpc/client/rpc_client.h"
#include "apie/rpc/init.h"
#include "apie/event/timer_impl.h"

namespace apie {


bool DAOFactory::registerFactory(const std::string name, TCreateMethod funcCreate)
{
	if (auto it = m_methods.find(name); it == m_methods.end())
	{
		// C++17 init-if
		m_methods[name] = funcCreate;
		return true;
	}

	return false;
}

std::shared_ptr<DeclarativeBase> DAOFactory::create(const std::string& name)
{
	if (auto it = m_methods.find(name); it != m_methods.end())
	{
		return it->second(); // call the createFunc
	}

	return nullptr;
}

std::map<std::string, DAOFactory::TCreateMethod>& DAOFactory::getMethods()
{
	return m_methods;
}

bool DAOFactory::addTable(const std::string& name, MysqlTable& table)
{
	auto findIte = m_tables.find(name);
	if (findIte != m_tables.end())
	{
		return false;
	}

	m_tables[name] = table;
	return true;
}

std::optional<MysqlTable> DAOFactory::getTable(const std::string& name)
{
	auto findIte = m_tables.find(name);
	if (findIte == m_tables.end())
	{
		return std::nullopt;
	}

	return std::make_optional(findIte->second);
}

bool DAOFactoryType::registerRequiredTable(DeclarativeBase::DBType type, const std::string& name, DAOFactory::TCreateMethod funcCreate)
{
	switch (type)
	{
	case DeclarativeBase::DBType::DBT_Account:
	{
		return DAOFactoryTypeSingleton::get().account.registerFactory(name, funcCreate);
	}
	case DeclarativeBase::DBType::DBT_Role:
	{
		return DAOFactoryTypeSingleton::get().role.registerFactory(name, funcCreate);
	}
	default:
	{
		return false;
	}
	}

	return false;
}

std::optional<std::map<std::string, DAOFactory::TCreateMethod>> DAOFactoryType::getRequiredTable(DeclarativeBase::DBType type)
{
	switch (type)
	{
	case DeclarativeBase::DBType::DBT_Account:
	{
		return std::make_optional(this->account.getMethods());
	}
	case DeclarativeBase::DBType::DBT_Role:
	{
		return std::make_optional(this->role.getMethods());
	}
	default:
		break;
	}
	return std::nullopt;
}

std::shared_ptr<DeclarativeBase> DAOFactoryType::createDAO(DeclarativeBase::DBType type, const std::string& name)
{
	switch (type)
	{
	case DeclarativeBase::DBType::DBT_Account:
	{
		return this->account.create(name);
	}
	case DeclarativeBase::DBType::DBT_Role:
	{
		return this->role.create(name);
	}
	default:
		break;
	}

	return nullptr;
}

bool DAOFactoryType::addLoadedTable(DeclarativeBase::DBType type, const std::string& name, MysqlTable& table)
{
	switch (type)
	{
	case DeclarativeBase::DBType::DBT_Account:
	{
		return this->account.addTable(name, table);
	}
	case DeclarativeBase::DBType::DBT_Role:
	{
		return this->role.addTable(name, table);
	}
	default:
		break;
	}

	return false;
}

DAOFactory* DAOFactoryType::getDAOFactory(DeclarativeBase::DBType type)
{
	switch (type)
	{
	case DeclarativeBase::DBType::DBT_Account:
	{
		return &account;
	}
	case DeclarativeBase::DBType::DBT_Role:
	{
		return &role;
	}
	default:
		break;
	}

	return nullptr;
}

bool CallMysqlDescTable(::rpc_msg::CHANNEL server, DeclarativeBase::DBType dbType, std::vector<std::string> tables, CallMysqlDescTableCB cb, uint64_t iCallCount)
{
	auto recallObj = CallMysqlDescTable;

	::mysql_proxy_msg::MysqlDescribeRequest args;
	for (const auto& table : tables)
	{
		auto ptrAdd = args.add_names();
		*ptrAdd = table;
	}

	iCallCount = iCallCount + 1;
	auto rpcCB = [server, dbType, tables, cb, recallObj, iCallCount](const apie::status::Status& status, const std::shared_ptr<::mysql_proxy_msg::MysqlDescribeResponse>& response) {
		if (!status.ok())
		{
			//ReSend
			std::stringstream ss;
			ss << "recall|iCallCount:" << iCallCount;

			ASYNC_PIE_LOG("LogicAsyncCallFunctor", PIE_CYCLE_DAY, PIE_WARNING, ss.str().c_str());
			auto ephemeralTimerCb = [server, dbType, tables, cb, recallObj, iCallCount]() mutable
			{
				recallObj(server, dbType, tables, cb, iCallCount);
			};
			auto ptrTimer = apie::event_ns::EphemeralTimerMgrSingleton::get().createEphemeralTimer(ephemeralTimerCb);
			ptrTimer->enableTimer(1000);
			return;
		}

		std::stringstream ss;
		ss << response->ShortDebugString();
		ASYNC_PIE_LOG("mysql_desc", PIE_CYCLE_DAY, PIE_DEBUG, ss.str().c_str());

		if (!response->result())
		{
			cb(false, response->error_info(), iCallCount);
			return;
		}

		for (auto tableData : response->tables())
		{
			MysqlTable table;
			table = DeclarativeBase::convertFrom(tableData.second);

			auto ptrDaoBase = DAOFactoryTypeSingleton::get().createDAO(dbType, tableData.first);
			if (ptrDaoBase == nullptr)
			{
				std::stringstream ss;
				ss << "tableName:" << tableData.first << " not declare";

				cb(false, ss.str(), iCallCount);
				return;
			}

			ptrDaoBase->initMetaData(table);
			bool bResult = ptrDaoBase->checkInvalid();
			if (!bResult)
			{
				std::stringstream ss;
				ss << "tableName:" << tableData.first << " checkInvalid false";

				cb(false, ss.str(), iCallCount);
				return;
			}

			DAOFactoryTypeSingleton::get().addLoadedTable(dbType, tableData.first, table);
		}

		cb(true, response->error_info(), iCallCount);
	};
	return apie::rpc::RPC_Call<::mysql_proxy_msg::MysqlDescribeRequest, ::mysql_proxy_msg::MysqlDescribeResponse>(server, rpc_msg::RPC_MysqlDescTable, args, rpcCB);
}

bool RegisterRequiredTable(const ::rpc_msg::CHANNEL& server, const std::map<std::string, DAOFactory::TCreateMethod> &loadTables, CallMysqlDescTableCB cb)
{
	auto type = static_cast<DeclarativeBase::DBType>(server.type());
	for (const auto& items : loadTables)
	{
		DAOFactoryTypeSingleton::get().registerRequiredTable(type, items.first, items.second);
	}

	auto requiredTableOpt = DAOFactoryTypeSingleton::get().getRequiredTable(type);
	if (!requiredTableOpt.has_value())
	{
		cb(true, "", 0);
		return true;
	}

	std::vector<std::string> tables;
	for (const auto& items : requiredTableOpt.value())
	{
		tables.push_back(items.first);
	}

	CallMysqlDescTable(server, type, tables, cb);

	return true;
}

}