#include "module_handler/dbproxy_mgr_module.h"

#include "logic/table_cache_mgr.h"

#include "../../common/dao/model_account.h"
#include "../../common/dao/model_account_name.h"
#include "../../common/dao/model_user.h"

namespace apie {

void DBProxyMgrModule::init()
{

	// PUBSUB
	auto& pubsub = apie::pubsub::PubSubManagerSingleton::get();
	pubsub.subscribe<::pubsub::LOGIC_CMD>(::pubsub::PUB_TOPIC::PT_LogicCmd, DBProxyMgrModule::PubSub_logicCmd);

	// CMD
	LogicCmdHandlerSingleton::get().init();

	// RPC
	auto& rpc = apie::rpc::RPCServerManagerSingleton::get();
	rpc.createRPCServer<::mysql_proxy_msg::MysqlDescribeRequest, ::mysql_proxy_msg::MysqlDescribeResponse>(rpc_msg::RPC_MysqlDescTable, DBProxyMgrModule::RPC_mysqlDescTable);
	rpc.createRPCServer<::mysql_proxy_msg::MysqlQueryRequest, ::mysql_proxy_msg::MysqlQueryResponse>(rpc_msg::RPC_MysqlQuery, DBProxyMgrModule::RPC_mysqlQuery);
	rpc.createRPCServer<::mysql_proxy_msg::MysqlInsertRequest, ::mysql_proxy_msg::MysqlInsertResponse>(rpc_msg::RPC_MysqlInsert, DBProxyMgrModule::RPC_mysqlInsert);
	rpc.createRPCServer<::mysql_proxy_msg::MysqlUpdateRequest, ::mysql_proxy_msg::MysqlUpdateResponse>(rpc_msg::RPC_MysqlUpdate, DBProxyMgrModule::RPC_mysqlUpdate);
	rpc.createRPCServer<::mysql_proxy_msg::MysqlDeleteRequest, ::mysql_proxy_msg::MysqlDeleteResponse>(rpc_msg::RPC_MysqlDelete, DBProxyMgrModule::RPC_mysqlDelete);
	rpc.createRPCServer<::mysql_proxy_msg::MysqlQueryRequestByFilter, ::mysql_proxy_msg::MysqlQueryResponse>(rpc_msg::RPC_MysqlQueryByFilter, DBProxyMgrModule::RPC_mysqlQueryByFilter);
	rpc.createRPCServer<::mysql_proxy_msg::MysqlMultiQueryRequest, ::mysql_proxy_msg::MysqlMulitQueryResponse>(rpc_msg::RPC_MysqlMultiQuery, DBProxyMgrModule::RPC_mysqlMultiQuery);

}



void DBProxyMgrModule::ready()
{

}



void DBProxyMgrModule::PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg)
{
	auto handlerOpt = LogicCmdHandlerSingleton::get().findCb(msg->cmd());
	if (!handlerOpt.has_value())
	{
		return;
	}

	handlerOpt.value()(*msg);
}

apie::status::Status DBProxyMgrModule::RPC_mysqlDescTable(
	const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlDescribeRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlDescribeResponse>& response)
{
	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		std::stringstream ss;
		ss << "LogicThread Null";

		response->set_result(false);
		response->set_error_info(ss.str());
		return { apie::status::StatusCode::OK, "" };
	}

	for (const auto& items : request->names())
	{
		mysql_proxy_msg::MysqlDescTable descTable;

		auto sharedPtrTable = TableCacheMgrSingleton::get().getTable(items);
		if (sharedPtrTable)
		{
			descTable = TableCacheMgrSingleton::get().convertFrom(sharedPtrTable);
			(*response->mutable_tables())[items] = descTable;
			continue;
		}

		std::stringstream ss;
		ss << "not cache:" << items;

		response->set_result(false);
		response->set_error_info(ss.str());

		return { apie::status::StatusCode::OK, "" };
	}

	response->set_result(true);
	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status DBProxyMgrModule::RPC_mysqlQuery(
	const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlQueryRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlQueryResponse>& response)
{
	std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(request->table_name());
	if (sharedTable == nullptr)
	{
		return { apie::status::StatusCode::INVALID_ARGUMENT, "CODE_TableNameNotExistError" };
	}

	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return { apie::status::StatusCode::INTERNAL, "CODE_LogicThreadNull" };
	}

	std::string sSQL;
	bool bResult = sharedTable->generateQuerySQL(ptrDispatched->getMySQLConnector(), *request, sSQL);
	response->set_sql_statement(sSQL);
	if (!bResult)
	{
		return { apie::status::StatusCode::INTERNAL, "CODE_GenerateQuerySQLError" };
	}

	std::shared_ptr<ResultSet> recordSet;
	bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);
	*response = DeclarativeBase::convertFrom(*sharedTable, recordSet);
	response->set_sql_statement(sSQL);
	response->set_result(bResult);
	if (!bResult)
	{
		response->set_error_info(ptrDispatched->getMySQLConnector().getError());
		return { apie::status::StatusCode::INTERNAL, "CODE_QueryError" };
	}

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status DBProxyMgrModule::RPC_mysqlMultiQuery(
	const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlMultiQueryRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlMulitQueryResponse>& multiResponse)
{
	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return { apie::status::StatusCode::INTERNAL, "CODE_LogicThreadNull" };
	}

	for (const auto& elems : request->requests())
	{
		::mysql_proxy_msg::MysqlQueryResponse response;

		std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(elems.table_name());
		if (sharedTable == nullptr)
		{
			return { apie::status::StatusCode::INVALID_ARGUMENT, "getTable Null" };
		}

		std::string sSQL;
		bool bResult = sharedTable->generateQuerySQL(ptrDispatched->getMySQLConnector(), elems, sSQL);
		response.set_sql_statement(sSQL);
		if (!bResult)
		{
			return { apie::status::StatusCode::INTERNAL, "getMySQLConnector Error" };
		}

		std::shared_ptr<ResultSet> recordSet;
		bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);
		response = DeclarativeBase::convertFrom(*sharedTable, recordSet);
		response.set_sql_statement(sSQL);
		response.set_result(bResult);
		if (!bResult)
		{
			response.set_error_info(ptrDispatched->getMySQLConnector().getError());
			auto ptrAdd = multiResponse->add_results();
			*ptrAdd = response;

			return { apie::status::StatusCode::INTERNAL, "CODE_QueryError" };
		}

		auto ptrAdd = multiResponse->add_results();
		*ptrAdd = response;
	}

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status DBProxyMgrModule::RPC_mysqlQueryByFilter(
	const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlQueryRequestByFilter>& request, std::shared_ptr<::mysql_proxy_msg::MysqlQueryResponse>& response)
{
	std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(request->table_name());
	if (sharedTable == nullptr)
	{
		return { apie::status::StatusCode::INVALID_ARGUMENT, "getTable Null" };
	}

	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return { apie::status::StatusCode::INTERNAL, "CODE_LogicThreadNull Null" };
	}

	std::string sSQL;
	bool bResult = sharedTable->generateQueryByFilterSQL(ptrDispatched->getMySQLConnector(), *request, sSQL);
	response->set_sql_statement(sSQL);
	if (!bResult)
	{
		return { apie::status::StatusCode::INTERNAL, "generateQueryByFilterSQL Error" };
	}

	std::shared_ptr<ResultSet> recordSet;
	bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);

	response->mutable_table()->set_db(sharedTable->getDb());
	response->mutable_table()->set_name(sharedTable->getTable());
	response->set_result(bResult);
	if (!bResult)
	{
		response->set_error_info(ptrDispatched->getMySQLConnector().getError());
	}

	if (!recordSet)
	{
		return { apie::status::StatusCode::OK, "" };
	}

	const uint32_t iBatchSize = 3;
	uint32_t iCurBatchSize = 0;
	uint32_t iOffset = 0;

	do
	{
		auto optRowData = DeclarativeBase::convertToRowFrom(*sharedTable, recordSet);
		if (optRowData.has_value())
		{
			auto ptrAddRows = response->mutable_table()->add_rows();
			*ptrAddRows = optRowData.value();

			iCurBatchSize++;
			if (iCurBatchSize >= iBatchSize)
			{
				iCurBatchSize = 0;

				apie::rpc::RPC_AsyncStreamReply(client, ::rpc_msg::CODE_Ok, response->SerializeAsString(), true, iOffset);
				response->mutable_table()->clear_rows();
			}
		}
		else
		{
			break;
		}

		iOffset++;
	} while (true);

	apie::rpc::RPC_AsyncStreamReply(client, ::rpc_msg::CODE_Ok, response->SerializeAsString(), false, iOffset);

	return { apie::status::StatusCode::OK_ASYNC, "" };
}


apie::status::Status DBProxyMgrModule::RPC_mysqlInsert(
	const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlInsertRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlInsertResponse>& response)
{
	std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(request->table_name());
	if (sharedTable == nullptr)
	{
		return { apie::status::StatusCode::INVALID_ARGUMENT, "CODE_TableNameNotExistError" };
	}

	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return { apie::status::StatusCode::INTERNAL, "CODE_LogicThread Null" };
	}

	std::string sSQL;
	bool bResult = sharedTable->generateInsertSQL(ptrDispatched->getMySQLConnector(), *request, sSQL);
	response->set_sql_statement(sSQL);
	if (!bResult)
	{
		return { apie::status::StatusCode::INTERNAL, "generateInsertSQL Error" };
	}

	std::shared_ptr<ResultSet> recordSet;
	bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);
	response->set_result(bResult);
	if (!bResult)
	{
		response->set_error_info(ptrDispatched->getMySQLConnector().getError());
	}
	response->set_affected_rows(ptrDispatched->getMySQLConnector().getAffectedRows());
	response->set_insert_id(ptrDispatched->getMySQLConnector().getInsertId());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status DBProxyMgrModule::RPC_mysqlUpdate(
	const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlUpdateRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlUpdateResponse>& response)
{
	std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(request->table_name());
	if (sharedTable == nullptr)
	{
		return { apie::status::StatusCode::INVALID_ARGUMENT, "getTable Null" };
	}

	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return { apie::status::StatusCode::INTERNAL, "CODE_LogicThreadNull" };
	}

	std::string sSQL;
	bool bResult = sharedTable->generateUpdateSQL(ptrDispatched->getMySQLConnector(), *request, sSQL);
	response->set_sql_statement(sSQL);
	if (!bResult)
	{
		return { apie::status::StatusCode::INTERNAL, "generateUpdateSQL Error" };
	}

	std::shared_ptr<ResultSet> recordSet;
	bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);
	response->set_result(bResult);
	if (!bResult)
	{
		response->set_error_info(ptrDispatched->getMySQLConnector().getError());
	}
	response->set_affected_rows(ptrDispatched->getMySQLConnector().getAffectedRows());
	response->set_insert_id(ptrDispatched->getMySQLConnector().getInsertId());

	return { apie::status::StatusCode::OK, "" };
}
apie::status::Status DBProxyMgrModule::RPC_mysqlDelete(
	const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlDeleteRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlDeleteResponse>& response)
{
	std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(request->table_name());
	if (sharedTable == nullptr)
	{
		return { apie::status::StatusCode::INVALID_ARGUMENT, "getTable Null" };
	}

	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return { apie::status::StatusCode::INTERNAL, "getLogicThread Null" };
	}

	std::string sSQL;
	bool bResult = sharedTable->generateDeleteSQL(ptrDispatched->getMySQLConnector(), *request, sSQL);
	response->set_sql_statement(sSQL);
	if (!bResult)
	{
		return { apie::status::StatusCode::INTERNAL, "generateDeleteSQL Error" };
	}

	std::shared_ptr<ResultSet> recordSet;
	bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);
	response->set_result(bResult);
	if (!bResult)
	{
		response->set_error_info(ptrDispatched->getMySQLConnector().getError());
	}
	response->set_affected_rows(ptrDispatched->getMySQLConnector().getAffectedRows());

	return { apie::status::StatusCode::OK, "" };
}


}

