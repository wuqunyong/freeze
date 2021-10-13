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

	class ModelServiceNode : public DeclarativeBase {
	public:
		struct db_fields {
			uint32_t service_realm = 0;
			uint32_t service_type = 0;
			uint32_t service_id = 0;
			std::string ip;
			uint32_t port = 0;
			std::string listeners_config;
			std::string mysql_config;
			std::string nats_config;
			std::string redis_config;
		};

		enum Fields
		{
			service_realm = 0,
			service_type,
			service_id,
			ip,
			port,
			listeners_config,
			mysql_config,
			nats_config,
			redis_config
		};

		ModelServiceNode(uint32_t realm, uint32_t type, uint32_t id)
		{
			this->fields.service_realm = realm;
			this->fields.service_type = type;
			this->fields.service_id = id;
			this->bindTable(DeclarativeBase::DBType::DBT_ConfigDb, getFactoryName());
		}

		DAO_DEFINE_TYPE_INTRUSIVE(ModelServiceNode, db_fields, service_node);
	};


}
