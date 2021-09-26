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


	class GatewayRole : public std::enable_shared_from_this<GatewayRole>
	{
	public:
		GatewayRole(uint64_t iRoleId, uint64_t iSerialNum);
		~GatewayRole();

		uint64_t getSerailNum();
		uint64_t getRoleId();

		void setMaskFlag(uint32_t iFlag);
		uint32_t getMaskFlag();

		bool addRequestPerUnit(uint64_t iValue);
		void close();



	public:
		static std::shared_ptr<GatewayRole> createGatewayRole(uint64_t iRoleId, uint64_t iSerialNum);

	private:
		void resetRequestPerUnit();
		void destroy();

	private:
		uint64_t m_iRoleId = 0;
		uint64_t m_iSerialNum = 0;
		uint32_t m_iMaskFlag = 0;

		uint64_t m_iRequests = 0;
		uint64_t m_iRequestPerUnit = 0;
		uint64_t m_iRequestUnitExpiresAt = 0;
	};
}
