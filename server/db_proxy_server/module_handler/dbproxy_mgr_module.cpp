#include "dbproxy_mgr_module.h"

#include "../logic/table_cache_mgr.h"


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


	//rpc.createRPCServer<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(rpc_msg::RPC_MysqlInsert, DBProxyMgrModule::RPC_handleMysqlInsert);
	//rpc.createRPCServer<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(rpc_msg::RPC_MysqlUpdate, DBProxyMgrModule::RPC_handleMysqlUpdate);
	//rpc.createRPCServer<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(rpc_msg::RPC_MysqlDelete, DBProxyMgrModule::RPC_handleMysqlDelete);
	//rpc.createRPCServer<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(rpc_msg::RPC_MysqlQueryByFilter, DBProxyMgrModule::RPC_handleMysqlQueryByFilter);
	//rpc.createRPCServer<rpc_msg::MSG_RPC_REQUEST_ECHO, rpc_msg::MSG_RPC_RESPONSE_ECHO>(rpc_msg::RPC_MysqlMultiQuery, DBProxyMgrModule::RPC_handleMysqlMultiQuery);

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
		return {apie::status::StatusCode::OK, ""};
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

std::tuple<uint32_t, std::string> DBProxyMgrModule::RPC_handleMysqlMultiQuery(const ::rpc_msg::CLIENT_IDENTIFIER& client, const ::mysql_proxy_msg::MysqlMultiQueryRequest& request)
{
	::mysql_proxy_msg::MysqlMulitQueryResponse multiResponse;


	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return std::make_tuple(::rpc_msg::CODE_LogicThreadNull, multiResponse.SerializeAsString());
	}

	for (const auto& elems : request.requests())
	{
		::mysql_proxy_msg::MysqlQueryResponse response;

		std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(elems.table_name());
		if (sharedTable == nullptr)
		{
			return std::make_tuple(::rpc_msg::CODE_TableNameNotExistError, multiResponse.SerializeAsString());
		}

		std::string sSQL;
		bool bResult = sharedTable->generateQuerySQL(ptrDispatched->getMySQLConnector(), elems, sSQL);
		response.set_sql_statement(sSQL);
		if (!bResult)
		{
			return std::make_tuple(::rpc_msg::CODE_GenerateQuerySQLError, multiResponse.SerializeAsString());
		}

		std::shared_ptr<ResultSet> recordSet;
		bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);
		response = DeclarativeBase::convertFrom(*sharedTable, recordSet);
		response.set_sql_statement(sSQL);
		response.set_result(bResult);
		if (!bResult)
		{
			response.set_error_info(ptrDispatched->getMySQLConnector().getError());
			auto ptrAdd = multiResponse.add_results();
			*ptrAdd = response;

			return std::make_tuple(::rpc_msg::CODE_QueryError, multiResponse.SerializeAsString());
		}

		auto ptrAdd = multiResponse.add_results();
		*ptrAdd = response;
	}

	return std::make_tuple(::rpc_msg::CODE_Ok, multiResponse.SerializeAsString());
}

//std::tuple<uint32_t, std::string> DBProxyMgrModule::RPC_handleMysqlQueryByFilter(const ::rpc_msg::CLIENT_IDENTIFIER& client, const ::mysql_proxy_msg::MysqlQueryRequestByFilter& request)
//{
//	::mysql_proxy_msg::MysqlQueryResponse response;
//
//	std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(request.table_name());
//	if (sharedTable == nullptr)
//	{
//		return std::make_tuple(::rpc_msg::CODE_ParseError, response.SerializeAsString());
//	}
//
//	auto ptrDispatched = CtxSingleton::get().getLogicThread();
//	if (ptrDispatched == nullptr)
//	{
//		return std::make_tuple(::rpc_msg::CODE_LogicThreadNull, response.SerializeAsString());
//	}
//
//	std::string sSQL;
//	bool bResult = sharedTable->generateQueryByFilterSQL(ptrDispatched->getMySQLConnector(), request, sSQL);
//	response.set_sql_statement(sSQL);
//	if (!bResult)
//	{
//		return std::make_tuple(::rpc_msg::CODE_ParseError, response.SerializeAsString());
//	}
//
//	std::shared_ptr<ResultSet> recordSet;
//	bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);
//
//	response.mutable_table()->set_db(sharedTable->getDb());
//	response.mutable_table()->set_name(sharedTable->getTable());
//	response.set_result(bResult);
//	if (!bResult)
//	{
//		response.set_error_info(ptrDispatched->getMySQLConnector().getError());
//	}
//
//	if (!recordSet)
//	{
//		return std::make_tuple(::rpc_msg::CODE_Ok, response.SerializeAsString());
//	}
//
//	const uint32_t iBatchSize = 3;
//	uint32_t iCurBatchSize = 0;
//	uint32_t iOffset = 0;
//
//	do
//	{
//		auto optRowData = DeclarativeBase::convertToRowFrom(*sharedTable, recordSet);
//		if (optRowData.has_value())
//		{
//			auto ptrAddRows = response.mutable_table()->add_rows();
//			*ptrAddRows = optRowData.value();
//
//			iCurBatchSize++;
//			if (iCurBatchSize >= iBatchSize)
//			{
//				iCurBatchSize = 0;
//
//				RPC::RpcServerSingleton::get().asyncStreamReply(client, ::rpc_msg::CODE_Ok, response.SerializeAsString(), true, iOffset);
//				response.mutable_table()->clear_rows();
//			}
//		}
//		else
//		{
//			break;
//		}
//
//		iOffset++;
//	} while (true);
//
//	RPC::RpcServerSingleton::get().asyncStreamReply(client, ::rpc_msg::CODE_Ok, response.SerializeAsString(), false, iOffset);
//	return std::make_tuple(::rpc_msg::CODE_Ok_Async, "");
//}
//

std::tuple<uint32_t, std::string> DBProxyMgrModule::RPC_handleMysqlInsert(const ::rpc_msg::CLIENT_IDENTIFIER& client, const ::mysql_proxy_msg::MysqlInsertRequest& request)
{
	::mysql_proxy_msg::MysqlInsertResponse response;

	std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(request.table_name());
	if (sharedTable == nullptr)
	{
		return std::make_tuple(::rpc_msg::CODE_ParseError, response.SerializeAsString());
	}

	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return std::make_tuple(::rpc_msg::CODE_LogicThreadNull, response.SerializeAsString());
	}

	std::string sSQL;
	bool bResult = sharedTable->generateInsertSQL(ptrDispatched->getMySQLConnector(), request, sSQL);
	response.set_sql_statement(sSQL);
	if (!bResult)
	{
		return std::make_tuple(::rpc_msg::CODE_ParseError, response.SerializeAsString());
	}

	std::shared_ptr<ResultSet> recordSet;
	bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);
	response.set_result(bResult);
	if (!bResult)
	{
		response.set_error_info(ptrDispatched->getMySQLConnector().getError());
	}
	response.set_affected_rows(ptrDispatched->getMySQLConnector().getAffectedRows());
	response.set_insert_id(ptrDispatched->getMySQLConnector().getInsertId());

	return std::make_tuple(::rpc_msg::CODE_Ok, response.SerializeAsString());
}

std::tuple<uint32_t, std::string> DBProxyMgrModule::RPC_handleMysqlUpdate(const ::rpc_msg::CLIENT_IDENTIFIER& client, const ::mysql_proxy_msg::MysqlUpdateRequest& request)
{
	::mysql_proxy_msg::MysqlUpdateResponse response;

	std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(request.table_name());
	if (sharedTable == nullptr)
	{
		return std::make_tuple(::rpc_msg::CODE_ParseError, response.SerializeAsString());
	}

	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return std::make_tuple(::rpc_msg::CODE_LogicThreadNull, response.SerializeAsString());
	}

	std::string sSQL;
	bool bResult = sharedTable->generateUpdateSQL(ptrDispatched->getMySQLConnector(), request, sSQL);
	response.set_sql_statement(sSQL);
	if (!bResult)
	{
		return std::make_tuple(::rpc_msg::CODE_ParseError, response.SerializeAsString());
	}

	std::shared_ptr<ResultSet> recordSet;
	bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);
	response.set_result(bResult);
	if (!bResult)
	{
		response.set_error_info(ptrDispatched->getMySQLConnector().getError());
	}
	response.set_affected_rows(ptrDispatched->getMySQLConnector().getAffectedRows());
	response.set_insert_id(ptrDispatched->getMySQLConnector().getInsertId());

	return std::make_tuple(::rpc_msg::CODE_Ok, response.SerializeAsString());
}

std::tuple<uint32_t, std::string> DBProxyMgrModule::RPC_handleMysqlDelete(const ::rpc_msg::CLIENT_IDENTIFIER& client, const ::mysql_proxy_msg::MysqlDeleteRequest& request)
{
	::mysql_proxy_msg::MysqlDeleteResponse response;

	std::shared_ptr<MysqlTable> sharedTable = TableCacheMgrSingleton::get().getTable(request.table_name());
	if (sharedTable == nullptr)
	{
		return std::make_tuple(::rpc_msg::CODE_ParseError, response.SerializeAsString());
	}

	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return std::make_tuple(::rpc_msg::CODE_LogicThreadNull, response.SerializeAsString());
	}

	std::string sSQL;
	bool bResult = sharedTable->generateDeleteSQL(ptrDispatched->getMySQLConnector(), request, sSQL);
	response.set_sql_statement(sSQL);
	if (!bResult)
	{
		return std::make_tuple(::rpc_msg::CODE_ParseError, response.SerializeAsString());
	}

	std::shared_ptr<ResultSet> recordSet;
	bResult = ptrDispatched->getMySQLConnector().query(sSQL.c_str(), sSQL.length(), recordSet);
	response.set_result(bResult);
	if (!bResult)
	{
		response.set_error_info(ptrDispatched->getMySQLConnector().getError());
	}
	response.set_affected_rows(ptrDispatched->getMySQLConnector().getAffectedRows());

	return std::make_tuple(::rpc_msg::CODE_Ok, response.SerializeAsString());
}

}

