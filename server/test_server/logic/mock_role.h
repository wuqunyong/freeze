#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>
#include <memory>
#include <set>

#include "apie.h"

#include "pb_init.h"


namespace apie {

	class MockRole;

	struct PendingResponse
	{
		using ResponseCB = std::function<void(MockRole* ptrRole, MessageInfo info, const std::string& msg)>;

		uint32_t id = 0;

		time_t request_time = 0;
		uint32_t request_opcode = 0;

		time_t response_time = 0;
		uint32_t response_opcode = 0;

		ResponseCB cb;
		uint32_t timeout = 1000;
		uint64_t expired_at_ms;
	};

	struct PendingNotify
	{
		using ResponseCB = std::function<void(MockRole* ptrRole, MessageInfo info, const std::string& msg)>;

		uint32_t id = 0;
		uint32_t response_opcode = 0;

		ResponseCB cb;
		uint32_t timeout = 1000;
		uint64_t expired_at_ms;
	};

	struct PendingRPC
	{
		using ResponseCB = std::function<void(MockRole* ptrRole, MessageInfo info, const std::string& msg)>;

		uint32_t id = 0;

		time_t request_time = 0;
		uint32_t request_opcode = 0;

		time_t response_time = 0;
		uint32_t response_opcode = 0;

		ResponseCB cb;
		uint32_t timeout = 1000;
		uint64_t expired_at_ms;
	};

	class MockRole : public std::enable_shared_from_this<MockRole>
	{
	public:
		using HandlerCb = std::function<void(MockRole& mockRole, ::pubsub::TEST_CMD& msg)>;
		using HandleResponseCB = std::function<void(MockRole* ptrRole, MessageInfo info, const std::string& msg)>;
		using HandleDataCB = std::function<void(MockRole* ptrRole, MessageInfo info, const std::string& msg)>;

		enum ConnectTarget
		{
			CT_None = 0,
			CT_Login = 1,
			CT_Gateway = 2,
		};

		MockRole(uint64_t iIggId);
		~MockRole();

		void setUp();
		void tearDown();

		void start();

		uint64_t getIggId();
		std::shared_ptr<ClientProxy> getClientProxy();
		void setClientProxy(std::shared_ptr<ClientProxy> ptrProxy);

		void processCmd();
		void addTimer(uint64_t interval);

		void disableCmdTimer();

		void clearMsg();
		void pushMsg(::pubsub::TEST_CMD& msg);

		bool addResponseHandler(uint32_t opcodes, HandleResponseCB cb);
		HandleResponseCB findResponseHandler(uint32_t opcodes);
		void clearResponseHandler();

		bool handleResponse(MessageInfo info, const std::string& msg);

		void setPauseProcess(bool flag);

		uint32_t waitResponse(uint32_t response, uint32_t request, HandleResponseCB cb = nullptr, uint32_t timeout = 6000);
		std::optional<PendingResponse> findWaitResponse(uint32_t response);
		void removeWaitResponse(uint32_t id);

		bool handleWaitResponse(MessageInfo info, const std::string& msg);


		void waitRPC(uint32_t iRPCId, uint32_t request, HandleResponseCB cb = nullptr, uint32_t timeout = 6000);
		std::optional<PendingRPC> findWaitRPC(uint32_t iRPCId);
		void removeWaitRPC(uint32_t id);

		bool handleWaitRPC(MessageInfo info, const std::string& msg);
		bool handleData(MessageInfo info, const std::string& msg);

		void clearWait();


		bool hasTimeout(uint64_t iCurMS);
		uint64_t sendMsg(uint32_t iOpcode, const ::google::protobuf::Message& msg);

		void setSSeqId(uint32_t iId);

		std::map<std::tuple<uint32_t, uint32_t>, std::vector<uint64_t>>& getReplyDelay();
		std::map<std::tuple<uint32_t, uint32_t>, std::tuple<uint64_t, uint64_t, uint64_t, uint64_t>>& getMergeReplyDelay();


	public:
		static std::shared_ptr<MockRole> createMockRole(uint64_t iIggId);

		static bool registerPbOpcodeName(uint32_t iOpcode, const std::string& sName);
		static std::optional<std::string> getPbNameByOpcode(uint32_t iOpcode);

		static bool addHandler(const std::string& sModule, const std::string& sCmd, HandlerCb cb);
		static HandlerCb findHandler(const std::string& sModule, const std::string& sCmd);

		static bool registerDataHandler(uint32_t opcodes, HandleDataCB cb);
		static HandleDataCB findDataHandler(uint32_t opcodes);

	private:
		void handleMsg(::pubsub::TEST_CMD& msg);
		
		void handle_MSG_GAMESERVER_LOGINRESP(MessageInfo info, const std::string& msg);
		

		void sendKeepAlive();

		void calculateCostTime(std::tuple<uint32_t, uint32_t> key, uint32_t iDelay);

	private:
		uint32_t m_id = 0;
		uint64_t m_iIggId;
		std::shared_ptr<ClientProxy> m_clientProxy;
		event_ns::TimerPtr m_cmdTimer;

		ConnectTarget m_target = CT_None;

		bool m_bInit = false;
		uint64_t m_iCurIndex = 0;
		std::vector<::pubsub::TEST_CMD> m_configCmd;

		bool m_bPauseProcess = false;

		std::map<uint32_t, HandleResponseCB> m_responseHandler;

		std::list<PendingResponse> m_pendingResponse;
		std::list<PendingRPC> m_pendingRPC;

		uint32_t m_iSSeqId = 0;
		uint64_t m_iNextKeepAliveTime = 0;

		std::map<std::tuple<uint32_t, uint32_t>, std::vector<uint64_t>>  m_replyDelay;  // key: request-response, value:delay(ms)
		std::map<std::tuple<uint32_t, uint32_t>, std::tuple<uint64_t, uint64_t, uint64_t, uint64_t>> m_mergeReplyDelay; // value:min,max,count,total

		static std::map<std::string, std::map<std::string, HandlerCb>> m_cmdHandler; // <module,<cmd,cb>>
		static std::map<uint32_t, HandleDataCB> m_dataHandler;
		static std::map<uint32_t, std::string> s_pbReflect;
	};
}
