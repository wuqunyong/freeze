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
#include "logic/role/role.h"

namespace apie {


	class GatewayRole : public std::enable_shared_from_this<GatewayRole>
	{
	public:
		GatewayRole(uint64_t iRoleId, uint64_t iSerialNum, RoleLoader::LoaderPtr ptrLoader);
		~GatewayRole();

		uint64_t getSerailNum();
		uint64_t getRoleId();

		void setMaskFlag(uint32_t iFlag);
		uint32_t getMaskFlag();

		bool addRequestPerUnit(uint64_t iValue);
		void close();

		RoleLoader::LoaderPtr getLoader();

	public:
		static std::shared_ptr<GatewayRole> createGatewayRole(uint64_t iRoleId, uint64_t iSerialNum, RoleLoader::LoaderPtr ptrLoader);

	private:
		void resetRequestPerUnit();
		void destroy();

	private:
		bool m_bSubNats = false;

		uint64_t m_iRoleId = 0;
		uint64_t m_iSerialNum = 0;
		uint32_t m_iMaskFlag = 0;

		uint64_t m_iRequests = 0;
		uint64_t m_iRequestPerUnit = 0;
		uint64_t m_iRequestUnitExpiresAt = 0;

		RoleLoader::LoaderPtr m_ptrLoader = nullptr;
	};
}
