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

		DAO_DEFINE_TYPE_INTRUSIVE(db_fields);

		ModelAccountName() = default;
		ModelAccountName(uint64_t account_id)
		{
			this->fields.account_id = account_id;
			this->bindTable(DeclarativeBase::DBType::DBT_Account, getFactoryName());
		}

		static std::shared_ptr<DeclarativeBase> createMethod()
		{
			return std::make_shared<ModelAccountName>();
		}

		static std::string getFactoryName() 
		{ 
			return "account_name"; 
		}
	};


}
