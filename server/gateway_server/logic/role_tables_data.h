#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>
#include <memory>

#include "apie.h"

#include "../../common/dao/model_user.h"
#include "../../common/dao/model_role_extra.h"

namespace apie {


	class RoleTablesData : public std::enable_shared_from_this<RoleTablesData>
	{
	public:
		using CallbackType = std::function<void(const status::Status& status)>;

		RoleTablesData(uint64_t roleId);
		~RoleTablesData();

		void LoadFromDb(CallbackType cb);
		bool SaveToDb(bool bFlush=false);

		bool onLoaded();
		bool onBeforeSave();

	public:
		uint64_t role_id;
		ModelUser user;
		ModelRoleExtra role_extra;

		::rpc_msg::CHANNEL server;
	};

	using RoleTablesDataPtr = std::shared_ptr<RoleTablesData>;
}
