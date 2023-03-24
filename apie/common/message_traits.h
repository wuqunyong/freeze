#pragma once

#include <string>
#include <cstdint>
#include <type_traits>
#include <concepts>

#include "apie/common/macros.h"
#include "apie/rpc/client/rpc_client.h"

#include "apie/mysql_driver/mysql_orm.h"
#include "apie/mysql_driver/dao_factory.h"
#include "apie/rpc/init.h"


namespace apie {

DEFINE_TYPE_TRAIT(HasInsertToDb, insertToDb)
DEFINE_TYPE_TRAIT(HasDeleteFromDb, deleteFromDb)
DEFINE_TYPE_TRAIT(HasUpdateToDb, updateToDb)
DEFINE_TYPE_TRAIT(HasLoadFromDb, loadFromDb)

template <typename T>
class HasDbSerializer {
 public:
  static constexpr bool value =
	  HasInsertToDb<T>::value && HasDeleteFromDb<T>::value &&
	  HasUpdateToDb<T>::value && HasLoadFromDb<T>::value;
};

// avoid potential ODR violation
template <typename T>
constexpr bool HasDbSerializer<T>::value;


template <typename T>
struct LoadFromDbCallback_
{
	using ReplyCallback = std::function<void(apie::status::Status, T& dbObj, uint32_t iRows)>;
}; 

template <typename T>
using LoadFromDbReplyCB = typename LoadFromDbCallback_<T>::ReplyCallback;


template <typename ...Ts>
struct LoadFromDbMultiCallback_
{
	using ReplyCallback = std::function<void(const apie::status::Status& status, std::tuple<Ts...>& tupleData, const std::array<uint32_t, sizeof...(Ts)>& rows)>;
};

template <typename ...Ts>
using LoadFromDbMultiReplyCB = typename LoadFromDbMultiCallback_<Ts...>::ReplyCallback;

template <typename T>
struct LoadFromDbByFilterCallback_
{
	using ReplyCallback = std::function<void(apie::status::Status, std::vector<T>& dbObjList)>;
};

template <typename T>
using LoadFromDbByFilterCB = typename LoadFromDbByFilterCallback_<T>::ReplyCallback;



using InsertToDbCB = std::function<void(apie::status::Status, bool result, uint64_t affectedRows, uint64_t insertId)>;
using UpdateToDbCB = std::function<void(apie::status::Status, bool result, uint64_t affectedRows)>;
using DeleteFromDbCB = std::function<void(apie::status::Status, bool result, uint64_t affectedRows)>;

// check: [0]:pendingNum, [1]:completedNum, [2]:doneFlag
using InsertDoneCb = std::function<void(const status::Status& status, const std::tuple<uint32_t, uint32_t>& insertRows)>;

template <typename T>
typename std::enable_if<std::is_base_of<DeclarativeBase, T>::value, bool>::type
InsertToDb(::rpc_msg::CHANNEL server, T& dbObj, InsertToDbCB cb)
{
	dbObj.dirtySet();

	mysql_proxy_msg::MysqlInsertRequest insertRequest = dbObj.generateInsert();
	dbObj.dirtyReset();

	auto insertCB = [cb](const apie::status::Status& status, const std::shared_ptr<::mysql_proxy_msg::MysqlInsertResponse>& response) mutable {
		if (!status.ok())
		{
			if (cb)
			{
				cb(status, false, 0, 0);
			}
			return;
		}

		std::stringstream ss;
		ss << response->ShortDebugString();
		ASYNC_PIE_LOG(PIE_DEBUG, "mysql_insert|{}", ss.str());

		if (cb)
		{
			apie::status::Status newStatus = status;
			if (!response->result())
			{
				newStatus.setErrorMessage(response->error_info());
			}

			cb(newStatus, response->result(), response->affected_rows(), response->insert_id());
		}
	};
	return apie::rpc::RPC_Call<::mysql_proxy_msg::MysqlInsertRequest, ::mysql_proxy_msg::MysqlInsertResponse>(server, ::rpc_msg::RPC_MysqlInsert, insertRequest, insertCB);
}


template <typename T>
typename std::enable_if<std::is_base_of<DeclarativeBase, T>::value, bool>::type
DeleteFromDb(::rpc_msg::CHANNEL server, T& dbObj, DeleteFromDbCB cb) 
{
	mysql_proxy_msg::MysqlDeleteRequest deleteRequest = dbObj.generateDelete();

	auto deleteCB = [cb](const apie::status::Status& status, const std::shared_ptr<::mysql_proxy_msg::MysqlDeleteResponse>& response) mutable {
		if (!status.ok())
		{
			if (cb)
			{
				cb(status, false, 0);
			}
			return;
		}


		std::stringstream ss;
		ss << response->ShortDebugString();
		ASYNC_PIE_LOG(PIE_DEBUG, "mysql_delete|{}", ss.str());

		if (cb)
		{
			cb(status, response->result(), response->affected_rows());
		}
	};
	return apie::rpc::RPC_Call<::mysql_proxy_msg::MysqlDeleteRequest, ::mysql_proxy_msg::MysqlDeleteResponse>(server, ::rpc_msg::RPC_MysqlDelete, deleteRequest, deleteCB);
}


template <typename T>
typename std::enable_if<std::is_base_of<DeclarativeBase, T>::value, bool>::type
UpdateToDb(::rpc_msg::CHANNEL server, T& dbObj, UpdateToDbCB cb)
{
	mysql_proxy_msg::MysqlUpdateRequest updateRequest = dbObj.generateUpdate();
	dbObj.dirtyReset();

	if (updateRequest.fields_size() == 0)
	{
		apie::status::Status status;
		status.setErrorCode(apie::status::StatusCode::DirtyFlagZero);

		if (cb)
		{
			cb(status, false, 0);
		}
		return false;
	}

	auto updateCB = [cb](const apie::status::Status& status, const std::shared_ptr<::mysql_proxy_msg::MysqlUpdateResponse>& response) mutable {
		if (!status.ok())
		{
			if (cb)
			{
				cb(status, false, 0);
			}

			return;
		}

		std::stringstream ss;
		ss << response->ShortDebugString();
		ASYNC_PIE_LOG(PIE_DEBUG, "mysql_update|{}", ss.str());

		if (cb)
		{
			cb(status, response->result(), response->affected_rows());
		}
	};
	return apie::rpc::RPC_Call<::mysql_proxy_msg::MysqlUpdateRequest, ::mysql_proxy_msg::MysqlUpdateResponse>(server, ::rpc_msg::RPC_MysqlUpdate, updateRequest, updateCB);
}


template<typename T>
concept HasLoadFromPb = requires(T a, const ::mysql_proxy_msg::MysqlRow & row)
{
	{ a.loadFromPb(row) } -> std::convertible_to<bool>;
};

template<class T>
concept LoadFromDBType = HasLoadFromPb<T> && std::is_base_of<DeclarativeBase, T>::value;


template <typename T>
requires LoadFromDBType<T>
bool LoadFromDb(::rpc_msg::CHANNEL server, T& dbObj, LoadFromDbReplyCB<T> cb)
{
	mysql_proxy_msg::MysqlQueryRequest queryRequest;
	queryRequest = dbObj.generateQuery();

	auto queryCB = [dbObj, cb](const apie::status::Status& status, const std::shared_ptr<::mysql_proxy_msg::MysqlQueryResponse>& response) mutable {
		if (!status.ok())
		{
			if (cb)
			{
				cb(status, dbObj, 0);
			}
			return;
		}

		std::stringstream ss;
		ss << response->ShortDebugString();
		ASYNC_PIE_LOG(PIE_DEBUG, "mysql_query|{}", ss.str());

		apie::status::Status newStatus;
		bool bResult = dbObj.loadFromPbCheck(*response);
		if (!bResult)
		{
			newStatus.setErrorCode(apie::status::StatusCode::LoadFromDbError);
			if (cb)
			{
				cb(newStatus, dbObj, 0);
			}
			return;
		}

		uint32_t iRowCount = response->table().rows_size();
		for (auto& rowData : response->table().rows())
		{
			dbObj.loadFromPb(rowData);
			break;
		}

		if (cb)
		{
			cb(status, dbObj, iRowCount);
		}
	};
	return apie::rpc::RPC_Call<::mysql_proxy_msg::MysqlQueryRequest, ::mysql_proxy_msg::MysqlQueryResponse>(server, ::rpc_msg::RPC_MysqlQuery, queryRequest, queryCB);
}

template <typename T>
requires LoadFromDBType<T>
bool LoadFromDbByFilter(::rpc_msg::CHANNEL server, T& dbObj, LoadFromDbByFilterCB<T> cb)
{
	auto ptrTuple = std::make_shared<std::tuple<std::vector<T>, bool>>();
	std::get<1>(*ptrTuple) = false;

	mysql_proxy_msg::MysqlQueryByFilterRequest queryRequest;
	queryRequest = dbObj.generateQueryByFilter();

	apie::rpc::RPCClientContext context(server);
	context.setType(rpc::RPCClientContext::Type::SERVER_STREAMING);
	
	auto queryCB = [dbObj, cb, ptrTuple](const apie::status::Status& status, const std::shared_ptr<::mysql_proxy_msg::MysqlQueryByFilterResponse>& response) mutable {
		auto& result = std::get<0>(*ptrTuple);
		auto& hasError = std::get<1>(*ptrTuple);
		if (hasError)
		{
			return;
		}

		if (!status.ok())
		{
			hasError = true;
			if (cb)
			{
				cb(status, result);
			}
			return;
		}

		std::stringstream ss;
		ss << response->ShortDebugString();
		ASYNC_PIE_LOG(PIE_DEBUG, "mysql_query|{}", ss.str());

		apie::status::Status newStatus;

		bool bResult = dbObj.loadFromPbCheck(*response);
		if (!bResult)
		{
			newStatus.setErrorCode(apie::status::StatusCode::LoadFromDbError);
			if (cb)
			{
				cb(newStatus, result);
			}
			return;
		}

		auto iBindType = dbObj.getDBType();
		auto sTableName = dbObj.getTableName();
		for (auto& rowData : response->table().rows())
		{
			typename std::remove_reference<decltype(dbObj)>::type newObj;

			bResult = newObj.bindTable(iBindType, sTableName);
			if (!bResult)
			{
				hasError = true;
				newStatus.setErrorCode(apie::status::StatusCode::DB_BindTableError);
				if (cb)
				{
					cb(newStatus, result);
				}
				return;
			}

			newObj.loadFromPb(rowData);
			result.push_back(newObj);
		}

		if (!status.hasMore())
		{
			if (cb)
			{
				cb(newStatus, result);
			}
		}
	};
	return apie::rpc::RPC_CallWithContext<::mysql_proxy_msg::MysqlQueryByFilterRequest, ::mysql_proxy_msg::MysqlQueryByFilterResponse>(context, ::rpc_msg::RPC_MysqlQueryByFilter, queryRequest, queryCB);
}

template <typename T>
requires LoadFromDBType<T>
bool LoadFromDbByQueryAll(::rpc_msg::CHANNEL server, T& dbObj, LoadFromDbByFilterCB<T> cb)
{
	auto ptrTuple = std::make_shared<std::tuple<std::vector<T>, bool>>();
	std::get<1>(*ptrTuple) = false;

	mysql_proxy_msg::MysqlQueryAllRequest queryRequest;
	queryRequest = dbObj.generateQueryAll();

	apie::rpc::RPCClientContext context(server);
	context.setType(rpc::RPCClientContext::Type::SERVER_STREAMING);

	auto queryCB = [dbObj, cb, ptrTuple](const apie::status::Status& status, const std::shared_ptr<::mysql_proxy_msg::MysqlQueryAllResponse>& response) mutable {
		auto& result = std::get<0>(*ptrTuple);
		auto& hasError = std::get<1>(*ptrTuple);
		if (hasError)
		{
			return;
		}

		if (!status.ok())
		{
			hasError = true;
			if (cb)
			{
				cb(status, result);
			}
			return;
		}

		std::stringstream ss;
		ss << response->ShortDebugString();
		ASYNC_PIE_LOG(PIE_DEBUG, "mysql_query|{}", ss.str());

		apie::status::Status newStatus;

		bool bResult = dbObj.loadFromPbCheck(*response);
		if (!bResult)
		{
			hasError = true;
			newStatus.setErrorCode(apie::status::StatusCode::LoadFromDbError);
			if (cb)
			{
				cb(newStatus, result);
			}
			return;
		}

		auto iBindType = dbObj.getDBType();
		auto sTableName = dbObj.getTableName();
		for (auto& rowData : response->table().rows())
		{
			typename std::remove_reference<decltype(dbObj)>::type newObj;

			bResult = newObj.bindTable(iBindType, sTableName);
			if (!bResult)
			{
				hasError = true;
				newStatus.setErrorCode(apie::status::StatusCode::DB_BindTableError);
				if (cb)
				{
					cb(newStatus, result);
				}
				return;
			}

			newObj.loadFromPb(rowData);
			result.push_back(newObj);
		}

		if (!status.hasMore())
		{
			if (cb)
			{
				cb(newStatus, result);
			}
		}
	};
	return apie::rpc::RPC_CallWithContext<::mysql_proxy_msg::MysqlQueryAllRequest, ::mysql_proxy_msg::MysqlQueryAllResponse>(context, ::rpc_msg::RPC_MysqlQueryAll, queryRequest, queryCB);
}

template<class Tuple, std::size_t N>
struct TupleDynamic {
	static void load(Tuple& t, ::mysql_proxy_msg::MysqlMulitQueryResponse& response, std::map<uint32_t, std::tuple<bool, uint32_t>>& loadResult)
	{
		TupleDynamic<Tuple, N - 1>::load(t, response, loadResult);

		std::tuple<uint32_t, uint32_t> result;

		auto &dbObj = std::get<N - 1>(t);
		auto ptrElems = response.mutable_results(N - 1);
		if (nullptr == ptrElems)
		{
			std::get<0>(result) = false;
			std::get<1>(result) = 0;
			loadResult[N - 1] = result;

			return;
		}

		auto &elmes = *ptrElems;
		bool bResult = dbObj.loadFromPbCheck(elmes);
		std::get<0>(result) = bResult;
		std::get<1>(result) = 0;

		if (bResult)
		{
			std::get<1>(result) = elmes.table().rows_size();
			for (auto& rowData : elmes.table().rows())
			{
				dbObj.loadFromPb(rowData);
				break;
			}
		}

		loadResult[N - 1] = result;
	}
};

template<class Tuple>
struct TupleDynamic<Tuple, 1> {
	static void load(Tuple& t, ::mysql_proxy_msg::MysqlMulitQueryResponse& response, std::map<uint32_t, std::tuple<bool, uint32_t>>& loadResult)
	{
		std::tuple<uint32_t, uint32_t> result;

		auto &dbObj = std::get<0>(t);
		auto ptrElems = response.mutable_results(0);
		if (nullptr == ptrElems)
		{
			std::get<0>(result) = false;
			std::get<1>(result) = 0;
			loadResult[0] = result;

			return;
		}

		auto &elmes = *ptrElems;
		bool bResult = dbObj.loadFromPbCheck(elmes);
		std::get<0>(result) = bResult;
		std::get<1>(result) = 0;

		if (bResult)
		{
			std::get<1>(result) = elmes.table().rows_size();;

			for (auto& rowData : elmes.table().rows())
			{
				dbObj.loadFromPb(rowData);
				break;
			}
		}

		loadResult[0] = result;
	}
};

template <typename... Ts>
typename std::enable_if<std::conjunction_v<std::is_base_of<DeclarativeBase, Ts>...>, bool>::type
Multi_LoadFromDb(LoadFromDbMultiReplyCB<Ts...> cb, ::rpc_msg::CHANNEL server, Ts... args)
{	
	static_assert((sizeof...(Ts)) > 0, "sizeof...(Ts) must > 0");
	static_assert((sizeof...(Ts)) < 20, "sizeof...(Ts) must < 20");

	constexpr uint32_t tupleSize = sizeof...(Ts);

	mysql_proxy_msg::MysqlMultiQueryRequest queryRequest;

	std::vector<mysql_proxy_msg::MysqlQueryRequest> v;
	(v.push_back(args.generateQuery()), ...);

	for (const auto& elem : v)
	{
		auto ptrAdd = queryRequest.add_requests();
		*ptrAdd = elem;
	}

	auto tupleData = std::make_tuple(args...);
	std::array<uint32_t, tupleSize> tupleRows = {0};

	auto queryCB = [cb, tupleData, tupleRows](const apie::status::Status& status, const std::shared_ptr<::mysql_proxy_msg::MysqlMulitQueryResponse>& response) mutable {
		if (!status.ok())
		{
			if (cb)
			{
				cb(status, tupleData, tupleRows);
			}
			return;
		}

		apie::status::Status newStatus;

		std::stringstream ss;
		ss << response->ShortDebugString();
		ASYNC_PIE_LOG(PIE_DEBUG, "mysql_multi_query|{}", ss.str());


		if (response->results_size() != std::tuple_size<decltype(tupleData)>::value)
		{
			newStatus.setErrorCode(apie::status::StatusCode::NotMatchedResultError);
			if (cb)
			{
				cb(newStatus, tupleData, tupleRows);
			}
			return;
		}

		std::map<uint32_t, std::tuple<bool, uint32_t>> loadResult;  // value: error,rows
		TupleDynamic<decltype(tupleData), std::tuple_size<decltype(tupleData)>::value>::load(tupleData, *response, loadResult);

		for (const auto& elems : loadResult)
		{
			tupleRows[elems.first] = std::get<1>(elems.second);
			if (!std::get<0>(elems.second))
			{
				newStatus.setErrorCode(apie::status::StatusCode::LoadFromDbError);
			}
		}

		if (cb)
		{
			cb(newStatus, tupleData, tupleRows);
		}
	};
	return apie::rpc::RPC_Call<::mysql_proxy_msg::MysqlMultiQueryRequest, ::mysql_proxy_msg::MysqlMulitQueryResponse>(server, ::rpc_msg::RPC_MysqlMultiQuery, queryRequest, queryCB);
}


//auto multiCb = [](const rpc_msg::STATUS& status, std::tuple<ModelAccount, ModelAccount, ModelAccount>& tupleData, std::array<uint32_t, 3>& tupleRows) {
//};
//bResult = Multi_LoadFromDb(multiCb, server, accountData, accountData, accountData);


//std::shared_ptr<std::tuple<uint32_t, uint32_t, bool>> ptrCheck : <0>需要插入的行数，<1>已插入的行数

template <size_t I = 0, typename... Ts>
void _Insert_OnNotExists(const ::rpc_msg::CHANNEL& server, std::tuple<Ts...>& tup, const std::array<uint32_t, sizeof...(Ts)>& rows, std::shared_ptr<std::tuple<uint32_t, uint32_t, bool>> ptrCheck, InsertDoneCb doneCb)
{
	// If we have iterated through all elements
	if constexpr (I == sizeof...(Ts))
	{
		// Last case, if nothing is left to
		// iterate, then exit the function
		if (std::get<0>(*ptrCheck) > std::get<1>(*ptrCheck))
		{
			return;
		}

		auto& doneFlag = std::get<2>(*ptrCheck);
		if (doneFlag)
		{
			return;
		}
		doneFlag = true;

		if (doneCb)
		{
			status::Status newStatus;
			doneCb(newStatus, std::make_tuple(std::get<0>(*ptrCheck), std::get<1>(*ptrCheck)));
		}
		return;
	}
	else
	{
		if (rows[I] == 0)
		{
			auto& pendingNum = std::get<0>(*ptrCheck);
			pendingNum += 1;

			auto cb = [ptrCheck, doneCb](status::Status status, bool result, uint64_t affectedRows, uint64_t insertId) mutable {
				auto& completedNum = std::get<1>(*ptrCheck);
				completedNum += 1;

				if (!status.ok() || !result)
				{
					if (!result)
					{
						status.setErrorCode(status::StatusCode::DB_InsertError);
					}

					auto& doneFlag = std::get<2>(*ptrCheck);
					if (doneFlag)
					{
						return;
					}
					doneFlag = true;

					if (doneCb)
					{
						doneCb(status, std::make_tuple(std::get<0>(*ptrCheck), std::get<1>(*ptrCheck)));
					}

					return;
				}

				if (std::get<0>(*ptrCheck) > std::get<1>(*ptrCheck))
				{
					return;
				}

				auto& doneFlag = std::get<2>(*ptrCheck);
				if (doneFlag)
				{
					return;
				}
				doneFlag = true;

				if (doneCb)
				{
					doneCb(status, std::make_tuple(std::get<0>(*ptrCheck), std::get<1>(*ptrCheck)));
				}
			};
			InsertToDb<typename std::tuple_element<I, std::decay_t<decltype(tup)>>::type>(server, std::get<I>(tup), cb);
		}

		// Going for next element.
		_Insert_OnNotExists<I + 1>(server, tup, rows, ptrCheck, doneCb);
	}
}

template <size_t I = 0, typename... Ts>
void Insert_OnNotExists(const ::rpc_msg::CHANNEL& server, std::tuple<Ts...>& tup, const std::array<uint32_t, sizeof...(Ts)>& rows, InsertDoneCb doneCb)
{
	std::shared_ptr<std::tuple<uint32_t, uint32_t, bool>> ptrInsertRows = std::make_shared<std::tuple<uint32_t, uint32_t, bool>>(0, 0, false);
	_Insert_OnNotExists(server, tup, rows, ptrInsertRows, doneCb);
}

template <size_t I = 0, typename... Ts>
void Update_OnChanged(const ::rpc_msg::CHANNEL& server, std::tuple<Ts...>& tup)
{
	// If we have iterated through all elements
	if constexpr (I == sizeof...(Ts))
	{
		// Last case, if nothing is left to
		// iterate, then exit the function
		return;
	}
	else
	{
		if (std::get<I>(tup).isDirty())
		{
			auto cb = [](status::Status status, bool result, uint64_t affectedRows) {
				if (!status.ok())
				{
					return;
				}
			};
			UpdateToDb<typename std::tuple_element<I, std::decay_t<decltype(tup)>>::type>(server, std::get<I>(tup), cb);
		}

		// Going for next element.
		Update_OnChanged<I + 1>(server, tup);
	}
}

template <size_t I = 0, typename... Ts>
void Update_OnForced(const ::rpc_msg::CHANNEL& server, std::tuple<Ts...>& tup)
{
	// If we have iterated through all elements
	if constexpr (I == sizeof...(Ts))
	{
		// Last case, if nothing is left to
		// iterate, then exit the function
		return;
	}
	else
	{
		std::get<I>(tup).dirtySet();
		auto cb = [](status::Status status, bool result, uint64_t affectedRows) {
			if (!status.ok())
			{
				return;
			}
		};
		UpdateToDb<typename std::tuple_element<I, std::decay_t<decltype(tup)>>::type>(server, std::get<I>(tup), cb);

		// Going for next element.
		Update_OnForced<I + 1>(server, tup);
	}
}


}  // namespace message

