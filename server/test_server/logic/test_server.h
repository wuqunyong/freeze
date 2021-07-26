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

#include "mock_role.h"


namespace apie {

class TestServerMgr
{
public:
	apie::status::Status init();
	apie::status::Status start();
	apie::status::Status ready();
	void exit();

	void addMockRole(std::shared_ptr<MockRole> ptrMockRole);
	std::shared_ptr<MockRole> findMockRole(uint64_t iRoleId);
	void removeMockRole(uint64_t iRoleId);

	void addSerialNumRole(uint64_t iSerialNum, uint64_t iRoleId);
	std::optional<uint64_t> findRoleIdBySerialNum(uint64_t iSerialNum);
	void removeSerialNum(uint64_t iSerialNum);

public:
	static void PubSub_logicCmd(const std::shared_ptr<::pubsub::LOGIC_CMD>& msg);

	static void Cmd_client(::pubsub::LOGIC_CMD& cmd);
	static void Cmd_autoTest(::pubsub::LOGIC_CMD& cmd);

	static void handleDefaultOpcodes(uint64_t serialNum, uint32_t opcodes, const std::string& msg);


public:
	std::shared_ptr<ClientProxy> m_ptrClientProxy;

	std::map<uint64_t, std::shared_ptr<MockRole>> m_mockRole;
	std::map<uint64_t, uint64_t> m_serialNumRole;
};


using TestServerMgrSingleton = ThreadSafeSingleton<TestServerMgr>;


}
