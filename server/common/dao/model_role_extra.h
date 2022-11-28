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

		ModelRoleExtra(uint64_t user_id)
		{
			this->fields.user_id = user_id;
			this->bindTable(DeclarativeBase::DBType::DBT_Role, getFactoryName());
		}

		DAO_DEFINE_TYPE_INTRUSIVE(ModelRoleExtra, db_fields, role_extra);
	};

	CYBER_REGISTER_COMPONENT(ModelRoleExtra)

}
