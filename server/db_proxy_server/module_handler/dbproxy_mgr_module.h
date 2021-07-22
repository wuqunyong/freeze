#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"


namespace apie {


class DBProxyMgrModule
{
public:
	static void init();
	static void ready();

public:
	// PUBSUB
	static void PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg);

	// RPC
	static apie::status::Status RPC_mysqlDescTable(
		const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlDescribeRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlDescribeResponse>& response);
	static apie::status::Status RPC_mysqlQuery(
		const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlQueryRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlQueryResponse>& response);
	static apie::status::Status RPC_mysqlInsert(
		const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlInsertRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlInsertResponse>& response);
	static apie::status::Status RPC_mysqlUpdate(
		const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlUpdateRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlUpdateResponse>& response);
	static apie::status::Status RPC_mysqlDelete(
		const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlDeleteRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlDeleteResponse>& response);
	static apie::status::Status RPC_mysqlQueryByFilter(
		const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlQueryRequestByFilter>& request, std::shared_ptr<::mysql_proxy_msg::MysqlQueryResponse>& response);
	static apie::status::Status RPC_mysqlMultiQuery(
		const ::rpc_msg::CLIENT_IDENTIFIER& client, const std::shared_ptr<::mysql_proxy_msg::MysqlMultiQueryRequest>& request, std::shared_ptr<::mysql_proxy_msg::MysqlMulitQueryResponse>& multiResponse);
};


}
