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

		virtual void* layoutAddress() override
		{
			return &fields;
		}

		virtual std::vector<uint32_t> layoutOffset() override
		{
			std::vector<uint32_t> layout = {
				offsetof(db_fields, user_id),
				offsetof(db_fields, extra_info),
			};

			return layout;
		}

		virtual std::vector<std::set<MysqlField::DB_FIELD_TYPE>> layoutType() override
		{
			std::vector<std::set<MysqlField::DB_FIELD_TYPE>> layout = {
				get_field_type(fields.user_id),
				get_field_type(fields.extra_info),
			};

			return layout;
		}

		static std::shared_ptr<DeclarativeBase> createMethod()
		{
			return std::make_shared<ModelRoleExtra>();
		}

		static std::string getFactoryName() 
		{ 
			return "role_extra"; 
		}

	public:
		db_fields fields;
	};


}
