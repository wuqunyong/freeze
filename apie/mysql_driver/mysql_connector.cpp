#include "apie/mysql_driver/mysql_connector.h"

#include <stdio.h>

#include <sstream>
#include <string>
#include "apie/network/logger.h"


MySQLConnector::MySQLConnector()
{
	this->initData();
}

MySQLConnector::~MySQLConnector(void)
{
	this->close();
}

void MySQLConnector::initData()
{
	this->re_connect_count_ = 0;
	this->mysql_ = nullptr;

	this->affected_rows_ = 0;
	this->insert_id_ = 0;
}

void MySQLConnector::init(MySQLConnectOptions& options)
{
	this->options_ = options;
}

bool MySQLConnector::connect(void)
{
	std::string sVersion = mysql_get_client_info();
	PIE_LOG(PIE_NOTICE, "startup|mysql version:{}", sVersion.c_str());

	MYSQL* mysql_con = mysql_init(NULL);
	if (!mysql_con)
	{
		std::stringstream ss;
		ss << "Could not init MySQL Connector to database " << this->options_.db;
		ASYNC_PIE_LOG(PIE_ERROR, "mysql|{}", ss.str().c_str());

		return false;
	}

	this->mysql_ = mysql_real_connect(mysql_con, this->options_.host.c_str(),this->options_.user.c_str(), 
		this->options_.passwd.c_str(),this->options_.db.c_str(), this->options_.port, NULL, CLIENT_MULTI_RESULTS);

	if (this->mysql_ != nullptr)
	{
		std::stringstream ss;

		bool my_true = true;
		auto result = mysql_autocommit(this->mysql_, my_true);

		//int rc = mysql_set_charset_name(this->mysql_, "utf8");

		std::string sSetNames("SET NAMES `utf8`");
		bool bResult = this->executeSQL(sSetNames.c_str(), (unsigned long)sSetNames.size());

		ss << "mysql_autocommit|" << result << "," << sSetNames << "|" << bResult;
		
		//ASYNC_PIE_LOG("mysql/mysql", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

		//std::string sSetCharacter("SET CHARACTER SET `utf8`");
		//this->executeSQL(sSetCharacter.c_str(),sSetCharacter.size());
		return true;
	}
	else
	{
		std::stringstream ss;
		ss << "sql: Connector failed. Reason was " << mysql_error(mysql_con);
		
		ASYNC_PIE_LOG(PIE_ERROR, "mysql|{}", ss.str().c_str());

		mysql_close(mysql_con);
		return false;
	}
}

void MySQLConnector::close(void)
{
	if (nullptr != this->mysql_)
	{
		mysql_close(this->mysql_);
		this->mysql_ = nullptr;
	}
}

bool MySQLConnector::dumpTableStructure(MYSQL_RES* pRES, MysqlTable& table)
{
	MYSQL_FIELD* fields = mysql_fetch_fields(pRES);
	if (fields == nullptr)
	{
		return false;
	}

	bool bSetDbName = false;

	uint32_t iFieldSize = mysql_num_fields(pRES);
	table.getFields().reserve(iFieldSize);

	uint32_t iOffset = 0;
	for (uint32_t i = 0; i < iFieldSize; i++)
	{
		if (!bSetDbName)
		{
			bSetDbName = true;
			table.setDb(std::string(fields[i].db, fields[i].db_length));
			table.setTable(std::string(fields[i].table, fields[i].table_length));
		}
		MysqlField field;
		field.setIndex(i);
		field.setName(fields[i].name, fields[i].name_length);
		field.setFlags(fields[i].flags);
		field.setType(fields[i].type);

		field.setOffset(iOffset);
		iOffset += field.getSize();

		table.appendField(field);
	}

	return true;
}


bool MySQLConnector::describeTable(const std::string tableName, MysqlTable& table)
{
	this->affected_rows_ = 0;
	this->insert_id_ = 0;
	this->error_ = "";

	if (NULL == this->mysql_)
	{
		return false;
	}
	else
	{
		char sql[2048] = {'\0'};
		snprintf(sql, 2048, "SELECT * FROM %s WHERE FALSE;", tableName.c_str());

		if (mysql_real_query(this->mysql_, sql, (unsigned long)strlen(sql)))
		{
			uint32_t last_errno = mysql_errno(this->mysql_);

			if (handleMySQLErrno(last_errno))  // If it returns true, an error was handled successfully (i.e. reconnection)
			{
				return describeTable(tableName, table);             // Try again
			}

			return false;
		}
		else
		{
			MYSQL_RES* ptr_mysql_res = mysql_store_result(this->mysql_);

			if (ptr_mysql_res != NULL)
			{
				bool bResult = dumpTableStructure(ptr_mysql_res, table);
				mysql_free_result(ptr_mysql_res);

				mysql_commit(this->mysql_);

				return bResult;
			}
			else // mysql_store_result() returned nothing; should it have?
			{
				return false;
			}
		}
	}
}

bool MySQLConnector::query(const char *q, unsigned long length, std::shared_ptr<ResultSet> &results, bool flags)
{
	this->affected_rows_ = 0;
	this->insert_id_ = 0;
	this->error_ = "";

	if (NULL == this->mysql_ || NULL == q)
	{
		return false;
	}
	else
	{
		if (mysql_real_query(this->mysql_, q, length))
		{
			uint32_t last_errno = mysql_errno(this->mysql_);

			if (handleMySQLErrno(last_errno))  // If it returns true, an error was handled successfully (i.e. reconnection)
			{
				return query(q, length, results, flags);             // Try again
			}

			return false;
		}
		else
		{
			MYSQL_RES* ptr_mysql_res = mysql_store_result(this->mysql_);

			if(ptr_mysql_res != NULL)
			{
				my_ulonglong num_rows = mysql_num_rows(ptr_mysql_res);
				if( num_rows > 0x00 )
				{
					auto ptrResults = new ResultSet(ptr_mysql_res);
					results.reset(ptrResults);
				}
				else
				{
					if(ptr_mysql_res != NULL)
					{
						mysql_free_result(ptr_mysql_res);
					}
				}

				if(flags)
				{
					mysql_commit(this->mysql_);
				}
			}
			else // mysql_store_result() returned nothing; should it have?
			{
				uint32_t query_field_count = mysql_field_count(this->mysql_);
				if(0 == query_field_count)
				{
					// query does not return data
					// (it was not a SELECT)
					affected_rows_ = mysql_affected_rows(this->mysql_);

					//std::cout << "affected_rows : " << affected_rows << std::endl;
					if (mysql_insert_id(this->mysql_) != 0)
					{
						insert_id_ = mysql_insert_id(this->mysql_);
					}
					
				}
				else // mysql_store_result() should have returned data
				{
					std::stringstream ss;
					ss << "Error: " << mysql_error(this->mysql_);
					
					ASYNC_PIE_LOG(PIE_ERROR, "mysql|{}", ss.str().c_str());

					return false;
				}
			}
		}
	}

	return true;
}

bool MySQLConnector::executeSQL(const char *q, unsigned long length)
{
	std::shared_ptr<ResultSet> record_set;
	bool bResult = this->query(q, length, record_set);
	return bResult;
}

std::string MySQLConnector::getError()
{
	return this->error_;
}

bool MySQLConnector::escapeString(const std::string& from, std::string& to)
{
    if (this->mysql_ == NULL)
    {
        return false;
    }

    size_t file_len = 2*from.size() + 1;
    char * chunk = new char [file_len];
    unsigned long real_len = mysql_real_escape_string(this->mysql_, chunk, from.c_str(), (unsigned long)from.size());

    //std::string str_image(chunk, real_len);
    //to = str_image;
	to.assign(chunk, real_len);

    delete [] chunk;

    return true;
}
bool MySQLConnector::handleMySQLErrno(uint32_t err_no)
{
	this->re_connect_count_++;

	if (!this->canTryReconnect())
	{
		return false;
	}

	switch (err_no)
	{
	case 2006:  // "MySQL server has gone away"
	case 2008:  // Client ran out of memory
	case 2013:  // "Lost connection to MySQL server during query"
	case 2048:  // "Invalid connection handle"
	case 2055:  // "Lost connection to MySQL server at '%s', system error: %d"
		{
			this->close();
			if (this->connect())
			{
				this->re_connect_count_ = 0;
				return true;
			}

			uint32_t last_errno = mysql_errno(getHandle());   // It's possible this attempted reconnect throws 2006 at us. To prevent crazy recursive calls, sleep here.

#ifdef WIN32
			Sleep(1000);
#else
			sleep(1);                                         // Sleep 1 seconds
#endif
			return handleMySQLErrno(last_errno);              // Call self (recursive)
		}

	default:
		std::stringstream ss;
		ss << "Unhandled MySQL errno " << err_no << " Unexpected behaviour possible. "  << "Error: " << mysql_error(this->mysql_) ;

		this->error_ = ss.str();
		//ASYNC_PIE_LOG("mysql/mysql", PIE_CYCLE_DAY, PIE_ERROR, this->error_.c_str());

		return false;
	}
}

bool MySQLConnector::canTryReconnect(void)
{
	if (this->re_connect_count_ > 10)
	{
		return false;
	}
	else
	{
		return true;
	}
}