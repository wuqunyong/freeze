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

	class ModelUser : public DeclarativeBase {
	public:
		struct db_fields {
			uint64_t user_id;
			uint64_t game_id = 1;
			uint32_t level = 2;
			int64_t register_time = 1;
			int64_t login_time = 2;
			int64_t offline_time = 3;
			std::string name = "hello";
		};

		enum Fields
		{
			user_id = 0,
			game_id,
			level,
			register_time,
			login_time,
			offline_time,
			name
		};

		ModelUser(uint64_t user_id)
		{
			this->fields.user_id = user_id;
			this->bindTable(DeclarativeBase::DBType::DBT_Role, getFactoryName());
		}

		DAO_DEFINE_TYPE_INTRUSIVE(ModelUser, db_fields, role_base);
	};


}
