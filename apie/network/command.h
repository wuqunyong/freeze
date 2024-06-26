#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <stdlib.h>

#include <event2/util.h>
#include <google/protobuf/message.h>

#include "apie/http/http_request.h"
#include "apie/http/http_response.h"
#include "apie/network/i_poll_events.hpp"
#include "apie/serialization/protocol_head.h"

namespace apie
{
	namespace service {
		class SyncServiceBase;
	}

	struct PassiveConnect
	{
		evutil_socket_t iFd;
		ProtocolType iType;
		std::string sIp;
		std::string sPeerIp;
		uint32_t iMaskFlag = 0;
		std::string sInfo;
	};

	struct MessageInfo
	{
	public:
		uint64_t iSessionId = 0;        // 会话ID
		uint32_t iRPCRequestID = 0;     // RPC_ID（RPC）
		uint32_t iOpcode = 0;           // 请求操作码
		uint32_t iResponseOpcode = 0;   // 响应操作码
		ConnetionType iConnetionType = ConnetionType::CT_NONE; // 连接类型
		ProtocolType iCodec = ProtocolType::PT_None;           // 协议类型

		int64_t iUserId = 0;
		uint32_t s_idSeq = 0;

		void setFlags(uint8_t iFlags)
		{
			iFlags_ = iFlags;
		}

		uint8_t getFlags()
		{
			return iFlags_;
		}

	private:
		uint8_t iFlags_ = 0;
	};

	struct PBRequest
	{
		ConnetionType type;
		MessageInfo info;
		std::shared_ptr<::google::protobuf::Message> ptrMsg;
	};

	struct PBForward
	{
		ConnetionType type;
		MessageInfo info;
		std::string sMsg;
	};

	struct SendData
	{
		ConnetionType type;
		uint64_t iSerialNum;
		std::string sData;
	};

	struct SyncSendData
	{
		ConnetionType type;
		uint64_t iSequenceNumber = 0;
		uint64_t iSerialNum;
		std::string sData;
		std::shared_ptr<apie::service::SyncServiceBase> ptrSyncBase = nullptr;
	};

	struct SendDataByFlag
	{
		ConnetionType type;
		uint64_t iSerialNum;
		ProtocolHead head;
		std::string sBody;
	};

	struct LogCmd
	{
		std::string sFile;
		int iCycle;
		int iLevel;
		std::string sMsg;
		bool bIgnoreMerge = false;
	};

	enum class DIAL_MODE
	{
		DM_ASYNC = 0,
		DM_SYNC = 1
	};

	struct DialParameters
	{
		DIAL_MODE mode = DIAL_MODE::DM_ASYNC;
		std::string sIp;
		uint16_t iPort;
		ProtocolType iCodecType;
		uint64_t iCurSerialNum = 0;
		uint32_t iMaskFlag = 0;
		std::shared_ptr<apie::service::SyncServiceBase> ptrSyncBase = nullptr;
		std::string sInfo;
	};

	struct DialResult
	{
		uint64_t iSerialNum;
		uint32_t iResult;
		std::string sLocalIp;
	};

	struct MetricData
	{
		std::string sMetric;
		std::map<std::string, std::string> tag;
		std::map<std::string, double> field;
	};

	struct CloseLocalClient
	{
		uint64_t iSerialNum;
	};

	struct CloseLocalServer
	{
		uint64_t iSerialNum;
	};

	struct ClientPeerClose
	{
		uint64_t iSerialNum;
		uint32_t iResult;
		std::string sInfo;
		uint32_t iActive;
	};

	struct ServerPeerClose
	{
		uint64_t iSerialNum;
		uint32_t iResult;
		std::string sInfo;
		uint32_t iActive;
	};
	
	struct SetServerSessionAttr
	{
		uint64_t iSerialNum;
		std::optional<std::string> optClientRandom;
		std::optional<std::string> optServerRandom;
		std::optional<std::string> optKey;
	};

	struct SetClientSessionAttr
	{
		uint64_t iSerialNum;
		std::optional<std::string> optKey;
	};

	struct LogicCmd
	{
		std::string sCmd;
	};

	struct LogicAsyncCallFunctor
	{
		std::function<void()> callFunctor;
	};

    //  This structure defines the commands that can be sent between threads.
    class Command
    {
	public:
		Command()
		{
			type = invalid_type;
		}

		enum type_t
		{
			invalid_type = 0,

			passive_connect,
			pb_reqeust,
			pb_forward,
			send_data,
			send_data_by_flag, // PH_COMPRESSED, PH_CRYPTO
			sync_send_data,
			dial,
			dial_result,
			set_server_session_attr,
			set_client_session_attr,

			async_log,
			metric_data,

			logic_cmd,
			logic_async_call_functor,

			close_local_client, //active(LogicThread -> IOThread | ClientProxy::sendClose)
			close_local_server,
			client_peer_close,  //client: passive(IOThread -> LogicThread)
			server_peer_close,  //server: passive(IOThread -> LogicThread)

			recv_http_request,  //server:passive_connect
			send_http_response, //server:passive_connect

			recv_http_response, //client:active_connect


			logic_start,
			logic_exit,
			stop_thread,

			done
        } type;

        union {
			struct {
			} invalid_type;

			struct {
				PassiveConnect* ptrData;
			} passive_connect;

			struct {
				PBRequest* ptrData;
			} pb_reqeust;

			struct {
				PBForward* ptrData;
			} pb_forward;

			struct {
				SendData* ptrData;
			} send_data;
			
			struct {
				SendDataByFlag* ptrData;
			} send_data_by_flag;

			struct {
				SyncSendData* ptrData;
			} sync_send_data;
			
			struct {
				LogCmd* ptrData;
			} async_log;

			struct {
				MetricData* ptrData;
			} metric_data;

			struct {
				DialParameters* ptrData;
			} dial;

			struct {
				DialResult* ptrData;
			} dial_result;

			struct {
				SetServerSessionAttr* ptrData;
			} set_server_session_attr;

			struct {
				SetClientSessionAttr* ptrData;
			} set_client_session_attr;

			struct {
				LogicCmd* ptrData;
			} logic_cmd;

			struct {
				LogicAsyncCallFunctor* ptrData;
			} logic_async_call_functor;

			struct {
				CloseLocalClient* ptrData;
			} close_local_client;

			struct {
				CloseLocalServer* ptrData;
			} close_local_server;

			struct {
				ClientPeerClose* ptrData;
			} client_peer_close;

			struct {
				ServerPeerClose* ptrData;
			} server_peer_close;

			struct {
				uint64_t iSerialNum;
				HttpRequest* ptrData;
			} recv_http_request;

			struct {
				uint64_t iSerialNum;
				HttpResponse* ptrData;
			} send_http_response;

			struct {
				uint64_t iSerialNum;
				HttpResponse* ptrData;
			} recv_http_response;

			struct {
				uint32_t iThreadId;
			} logic_start;

			struct {
				uint32_t iThreadId;
			} logic_exit;

			struct {
				uint32_t iThreadId;
			} stop_thread;

			struct {
			} done;

        } args;
    };

    //  Function to deallocate dynamically allocated components of the command.
    void deallocateCommand(Command* cmd);
}    

