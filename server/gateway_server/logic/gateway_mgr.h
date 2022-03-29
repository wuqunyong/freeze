#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"

namespace apie {

	class GatewayRole;

	struct PendingLoginRole
	{
		uint64_t role_id;
		std::string session_key;
		uint32_t db_id;
		uint64_t expires_at;
	};

	class GatewayMgr
	{
	public:
		GatewayMgr(std::string name, module_loader::ModuleLoaderBase* prtLoader);

		static std::string moduleName();
		static uint32_t modulePrecedence();

		apie::status::Status init();
		apie::status::Status start();
		apie::status::Status ready();
		apie::status::Status exit();

		void setHookReady(hook::HookPoint point);

		std::shared_ptr<GatewayRole> findGatewayRoleById(uint64_t iRoleId);
		std::shared_ptr<GatewayRole> findGatewayRoleBySerialNum(uint64_t iSerialNum);
		std::optional<uint64_t> findRoleIdBySerialNum(uint64_t iSerialNum);

		void addPendingRole(const PendingLoginRole &role);
		std::optional<PendingLoginRole> getPendingRole(uint64_t iRoleId);
		void removePendingRole(uint64_t iRoleId);

		bool addGatewayRole(std::shared_ptr<GatewayRole> ptrGatewayRole);
		bool removeGateWayRole(uint64_t iRoleId);


	private:
		std::string m_name;
		module_loader::ModuleLoaderBase* m_prtLoader;

		std::map<uint64_t, std::shared_ptr<GatewayRole>> m_serialNumMap; // key:serialNum, value:shared_ptr
		std::map<uint64_t, uint64_t> m_roleIdMapSerialNum;               // key:roleId, value:serialNum

		std::unordered_map<uint64_t, PendingLoginRole> m_pendingRole;
	};
}
