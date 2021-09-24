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

	class ModelRoleExtra : public DeclarativeBase {
	public:
		struct db_fields {
			uint64_t user_id;
			std::string extra_info = "hello";
		};

		enum Fields
		{
			user_id = 0,
			extra_info
		};

		DAO_DEFINE_TYPE_INTRUSIVE(db_fields);


		ModelRoleExtra() = default;
		ModelRoleExtra(uint64_t user_id)
		{
			this->fields.user_id = user_id;
			this->bindTable(DeclarativeBase::DBType::DBT_Role, getFactoryName());
		}

		static std::shared_ptr<DeclarativeBase> createMethod()
		{
			return std::make_shared<ModelRoleExtra>();
		}

		static std::string getFactoryName() 
		{ 
			return "role_extra"; 
		}
	};


}
