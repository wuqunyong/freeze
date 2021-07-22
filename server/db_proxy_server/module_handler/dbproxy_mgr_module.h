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




		static std::tuple<uint32_t, std::string> RPC_handleMysqlInsert(const ::rpc_msg::CLIENT_IDENTIFIER& client, const ::mysql_proxy_msg::MysqlInsertRequest& request);
		static std::tuple<uint32_t, std::string> RPC_handleMysqlUpdate(const ::rpc_msg::CLIENT_IDENTIFIER& client, const ::mysql_proxy_msg::MysqlUpdateRequest& request);
		static std::tuple<uint32_t, std::string> RPC_handleMysqlDelete(const ::rpc_msg::CLIENT_IDENTIFIER& client, const ::mysql_proxy_msg::MysqlDeleteRequest& request);
		//static std::tuple<uint32_t, std::string> RPC_handleMysqlQueryByFilter(const ::rpc_msg::CLIENT_IDENTIFIER& client, const ::mysql_proxy_msg::MysqlQueryRequestByFilter& request);
		static std::tuple<uint32_t, std::string> RPC_handleMysqlMultiQuery(const ::rpc_msg::CLIENT_IDENTIFIER& client, const ::mysql_proxy_msg::MysqlMultiQueryRequest& request);
	};

}
