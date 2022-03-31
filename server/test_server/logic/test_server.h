#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"

#include "../../pb_msg/business/login_msg.pb.h"
#include "../../pb_msg/business/role_server_msg.pb.h"

#include "logic/mock_role.h"


namespace apie {


class TestServerMgr
{
public:
	TestServerMgr(std::string name, module_loader::ModuleLoaderBase* prtLoader);

	static std::string moduleName();
	static uint32_t modulePrecedence();

	apie::status::Status init();
	apie::status::Status start();
	apie::status::Status ready();
	apie::status::Status exit();

	void setHookReady(hook::HookPoint point);

	void addMockRole(std::shared_ptr<MockRole> ptrMockRole);
	std::shared_ptr<MockRole> findMockRole(uint64_t iRoleId);
	void removeMockRole(uint64_t iRoleId);

	void addSerialNumRole(uint64_t iSerialNum, uint64_t iRoleId);
	std::optional<uint64_t> findRoleIdBySerialNum(uint64_t iSerialNum);
	void removeSerialNum(uint64_t iSerialNum);


public:
	std::string m_name;
	module_loader::ModuleLoaderBase* m_prtLoader;

	std::shared_ptr<ClientProxy> m_ptrClientProxy;

	std::map<uint64_t, std::shared_ptr<MockRole>> m_mockRole;
	std::map<uint64_t, uint64_t> m_serialNumRole;
};



}
