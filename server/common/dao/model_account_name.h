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

	class ModelAccountName : public DeclarativeBase {
	public:
		struct db_fields {
			uint64_t account_id = 0;
			std::string name;
		};

		enum Fields
		{
			account_id = 0,
			name,
		};

		ModelAccountName(uint64_t account_id)
		{
			this->fields.account_id = account_id;
			this->bindTable(DeclarativeBase::DBType::DBT_Account, getFactoryName());
		}

		DAO_DEFINE_TYPE_INTRUSIVE(ModelAccountName, db_fields, account_name);
	};


}
