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

	class ModelAccount : public DeclarativeBase {
	public:
		struct db_fields {
			uint64_t account_id = 0;
			uint32_t db_id = 0;
			int64_t register_time = 0;
			int64_t modified_time = 0;
		};

		enum Fields
		{
			account_id = 0,
			db_id,
			register_time,
			modified_time,
		};

		ModelAccount(uint64_t account_id)
		{
			this->fields.account_id = account_id;
			this->bindTable(DeclarativeBase::DBType::DBT_Account, getFactoryName());
		}

		DAO_DEFINE_TYPE_INTRUSIVE(ModelAccount, db_fields, account);
	};


}
