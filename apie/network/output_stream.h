#pragma once

#include <array>
#include <cstdint>
#include <string>

#include <event2/util.h>
#include <google/protobuf/message.h>

#include "apie/network/address.h"
#include "apie/network/windows_platform.h"
#include "apie/network/i_poll_events.hpp"
#include "apie/proto/init.h"
#include "apie/event/dispatcher_impl.h"
#include "apie/network/ctx.h"
#include "apie/serialization/protocol_head.h"
#include "apie/rpc/client/rpc_client.h"
#include "apie/network/command.h"

namespace apie {
	class Command;
	class Ctx;

namespace network {

	class OutputStream
	{
	public:
		static bool sendMsg(MessageInfo info, const ::google::protobuf::Message& msg);
		static bool sendMsgWithFlag(MessageInfo info, const ::google::protobuf::Message& msg);
		static bool sendProtobufMsgImpl(MessageInfo info, const ::google::protobuf::Message& msg);

		static bool sendMsgByStr(MessageInfo info, const std::string& msg);
		static bool sendMsgByStrWithFlag(MessageInfo info, const std::string& msg);
		static bool sendStringMsgImpl(MessageInfo info, const std::string& msg);


		static bool sendCommand(ConnetionType type, uint64_t iSerialNum, apie::Command& cmd);


		static bool sendPBMsgHead(MessageInfo info, const ::google::protobuf::Message& msg);
		static bool sendPBMsgUser(MessageInfo info, const ::google::protobuf::Message& msg);
	};

	// 只有客户端能发送同步请求
	template <typename Response>
	std::shared_future<std::shared_ptr<Response>> syncSendProtobufMsgImpl(MessageInfo info, const ::google::protobuf::Message& msg)
	{
		std::shared_future<std::shared_ptr<Response>> invalidResult;

		auto ptrConnection = event_ns::DispatcherImpl::getClientConnection(info.iSessionId);
		if (ptrConnection == nullptr)
		{
			return invalidResult;
		}

		uint32_t iThreadId = ptrConnection->getTId();
		auto ptrThread = apie::CtxSingleton::get().getThreadById(iThreadId);
		if (ptrThread == nullptr)
		{
			return invalidResult;
		}

		ProtocolHead head;
		head.iSeqNum = info.iRPCRequestID;
		head.iFlags = 0;
		head.iOpcode = info.iOpcode;
		head.iBodyLen = (uint32_t)msg.ByteSizeLong();

		auto ptrSync = std::make_shared<apie::service::SyncService<Response>>();

		SyncSendData* itemObjPtr = new SyncSendData;
		itemObjPtr->type = apie::ConnetionType::CT_CLIENT;
		itemObjPtr->iSequenceNumber = info.iRPCRequestID;
		itemObjPtr->iSerialNum = info.iSessionId;
		itemObjPtr->sData.append(reinterpret_cast<char*>(&head), sizeof(ProtocolHead));
		itemObjPtr->sData.append(msg.SerializeAsString());
		itemObjPtr->ptrSyncBase = ptrSync;

		Command command;
		command.type = Command::sync_send_data;
		command.args.sync_send_data.ptrData = itemObjPtr;
		ptrThread->push(command);

		return ptrSync->getFuture();
	}
}
}
