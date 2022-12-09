#pragma once

#include "apie/network/windows_platform.h"

#include <stdio.h>
#include <time.h>

#include <iostream>
#include <optional>
#include <tuple>
#include <stdexcept>
#include <iosfwd>
#include <sstream>
#include <bitset>
#include <vector>

#include <google/protobuf/message.h>

#include "apie/mysql_driver/mysql_table.h"
#include "apie/mysql_driver/mysql_connector.h"
#include "apie/mysql_driver/result_set.h"

#include "apie/proto/init.h"
#include "apie/status/status.h"


#ifdef _MSC_VER
#define PACKED_STRUCT(definition, ...)                                                             \
  __pragma(pack(push, 1)) definition, ##__VA_ARGS__;                                               \
  __pragma(pack(pop))
#else
#define PACKED_STRUCT(definition, ...) definition, ##__VA_ARGS__
#endif


//#define PACKED_STRUCT(definition, ...) definition, ##__VA_ARGS__ __attribute__((packed))

class DeclarativeBase
{
public:
	enum class DBType
	{
		DBT_None = 0,
		DBT_Account = 1,
		DBT_Role = 2,
		DBT_ConfigDb = 3,
	};

	//virtual uint32_t blockSize() = 0;
	virtual void* layoutAddress() = 0;
	virtual std::vector<uint32_t> layoutOffset() = 0;
	virtual std::vector<std::set<MysqlField::DB_FIELD_TYPE>> layoutType() = 0;
	virtual std::string getFieldName(uint32_t iIndex) = 0;

	template <typename T>
	void extract(T& t, unsigned char* address)
	{
		t = (*((T*)address));
	};

	template <typename T>
	void writeValue(void* address, T value)
	{
		*((T*)address) = value;
	}

	DBType getDBType();
	void setDBType(DeclarativeBase::DBType type);

	bool initMetaData(MysqlTable& table);
	bool bindTable(DeclarativeBase::DBType type, const std::string& name);

	std::string getTableName();

	bool loadFromDb(std::shared_ptr<ResultSet> sharedPtr);

	bool loadFromPbCheck(::mysql_proxy_msg::MysqlQueryResponse& response);
	bool loadFromPb(const ::mysql_proxy_msg::MysqlRow& row);

	bool isValid();

	std::optional<::mysql_proxy_msg::MysqlValue> getValueByIndex(uint32_t index);
	uint32_t getLayoutOffset(uint32_t index);

	size_t columNums();
	uint32_t fieldSize(uint32_t index);
	uint32_t getRowCount();

	void markDirty(const std::vector<uint8_t>& index);
	bool isDirty(uint8_t index);
	bool isDirty();
	void dirtySet();
	void dirtyReset();

	void markFilter(const std::vector<uint8_t>& index);
	bool isFilter(uint8_t index);
	void filterReset();

	std::string query(MySQLConnector& connector);

	mysql_proxy_msg::MysqlQueryRequest generateQuery();
	mysql_proxy_msg::MysqlInsertRequest generateInsert();
	mysql_proxy_msg::MysqlUpdateRequest generateUpdate();
	mysql_proxy_msg::MysqlDeleteRequest generateDelete();

	mysql_proxy_msg::MysqlQueryRequestByFilter generateQueryByFilter();
	mysql_proxy_msg::MysqlQueryAllRequest generateQueryAll();

	MysqlTable& getMysqlTable();

private:
	//bool loadFromPb(::mysql_proxy_msg::MysqlQueryResponse& response);

public:
	static std::string toString(MySQLConnector& connector, const ::mysql_proxy_msg::MysqlValue& value);

	static MysqlTable convertFrom(::mysql_proxy_msg::MysqlDescTable& desc);
	static mysql_proxy_msg::MysqlQueryResponse convertFrom(MysqlTable& table, std::shared_ptr<ResultSet> sharedPtr);
	static std::optional<mysql_proxy_msg::MysqlRow> convertToRowFrom( MysqlTable& table, std::shared_ptr<ResultSet> sharedPtr);


private:
	DBType m_dbType = DBType::DBT_None;

	MysqlTable m_table;
	std::bitset<256> m_dirtyFlags;
	std::bitset<256> m_filterFlags;
	uint32_t m_rowCount = 0;
	bool m_binded = false;
};

template<typename T>
apie::status::Status syncLoadDbByFilter(MySQLConnector& connector, T& obj, std::vector<T>& loadedList)
{
	auto queryFilter = obj.generateQueryByFilter();

	std::string sSQL;
	bool bResult = obj.getMysqlTable().generateQueryByFilterSQL(connector, queryFilter, sSQL);
	if (!bResult)
	{
		return { apie::status::StatusCode::INTERNAL, "generateQueryByFilterSQL Error" };
	}

	std::shared_ptr<ResultSet> recordSet;
	bResult = connector.query(sSQL.c_str(), sSQL.length(), recordSet);
	if (!bResult)
	{
		return { apie::status::StatusCode::INTERNAL, connector.getError() };
	}

	if (!recordSet)
	{
		return { apie::status::StatusCode::OK, "" };
	}

	uint32_t iRows = 0;
	do
	{
		auto optRowData = DeclarativeBase::convertToRowFrom(obj.getMysqlTable(), recordSet);
		if (optRowData.has_value())
		{
			iRows++;

			T newObj;
			newObj.loadFromPb(optRowData.value());
			loadedList.push_back(newObj);
		}
		else
		{
			break;
		}
	} while (true);

	return { apie::status::StatusCode::OK, "" };
}
