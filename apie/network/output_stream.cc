#include "apie/network/output_stream.h"

#include <array>
#include <cstdint>
#include <string>
#include <assert.h>


namespace apie {
namespace network {

	bool OutputStream::sendMsg(MessageInfo info, const ::google::protobuf::Message& msg)
	{
		info.setFlags(0);
		return sendProtobufMsgImpl(info, msg);
	}

	bool OutputStream::sendMsgWithFlag(MessageInfo info, const ::google::protobuf::Message& msg)
	{
		return sendProtobufMsgImpl(info, msg);
	}

	bool OutputStream::sendProtobufMsgImpl(MessageInfo info, const ::google::protobuf::Message& msg)
	{
		uint32_t iThreadId = 0;
		
		ConnetionType type = info.iConnetionType;

		switch (type)
		{
		case apie::ConnetionType::CT_NONE:
		{
			auto ptrConnection = event_ns::DispatcherImpl::getConnection(info.iSessionId);
			if (ptrConnection == nullptr)
			{
				auto ptrClient = event_ns::DispatcherImpl::getClientConnection(info.iSessionId);
				if (ptrClient == nullptr)
				{
					return false;
				}
				else
				{
					iThreadId = ptrClient->getTId();
					type = ConnetionType::CT_CLIENT;
				}
			}
			else
			{
				iThreadId = ptrConnection->getTId();
				type = ConnetionType::CT_SERVER;
			}
			break;
		}
		case apie::ConnetionType::CT_SERVER:
		{
			auto ptrConnection = event_ns::DispatcherImpl::getConnection(info.iSessionId);
			if (ptrConnection == nullptr)
			{
				return false;
			}

			iThreadId = ptrConnection->getTId();
			break;
		}
		case apie::ConnetionType::CT_CLIENT:
		{
			auto ptrConnection = event_ns::DispatcherImpl::getClientConnection(info.iSessionId);
			if (ptrConnection == nullptr)
			{
				return false;
			}

			iThreadId = ptrConnection->getTId();
			break;
		}
		default:
			break;
		}

		auto ptrThread = CtxSingleton::get().getThreadById(iThreadId);
		if (ptrThread == nullptr)
		{
			return false;
		}

		ProtocolHead head;
		head.iSeqNum = info.iSeqNum;
		head.iFlags = info.getFlags();
		head.iOpcode = info.iOpcode;
		head.iBodyLen = (uint32_t)msg.ByteSizeLong();

		if (info.getFlags() == 0)
		{
			SendData* itemObjPtr = new SendData;
			itemObjPtr->type = type;
			itemObjPtr->iSerialNum = info.iSessionId;
			itemObjPtr->sData.append(reinterpret_cast<char*>(&head), sizeof(ProtocolHead));
			itemObjPtr->sData.append(msg.SerializeAsString());

			Command command;
			command.type = Command::send_data;
			command.args.send_data.ptrData = itemObjPtr;
			ptrThread->push(command);
		} 
		else
		{
			SendDataByFlag* itemObjPtr = new SendDataByFlag;
			itemObjPtr->type = type;
			itemObjPtr->iSerialNum = info.iSessionId;
			itemObjPtr->head = head;
			itemObjPtr->sBody = msg.SerializeAsString();

			Command command;
			command.type = Command::send_data_by_flag;
			command.args.send_data_by_flag.ptrData = itemObjPtr;
			ptrThread->push(command);
		}

		return true;
	}

	bool OutputStream::sendMsgByStr(MessageInfo info, const std::string& msg)
	{
		info.setFlags(0);
		return sendStringMsgImpl(info, msg);
	}

	bool OutputStream::sendMsgByStrWithFlag(MessageInfo info, const std::string& msg)
	{
		return sendStringMsgImpl(info, msg);
	}

	bool OutputStream::sendStringMsgImpl(MessageInfo info, const std::string& msg)
	{
		uint32_t iThreadId = 0;

		auto type = info.iConnetionType;
		switch (type)
		{
		case apie::ConnetionType::CT_NONE:
		{
			auto ptrConnection = event_ns::DispatcherImpl::getConnection(info.iSessionId);
			if (ptrConnection == nullptr)
			{
				auto ptrClient = event_ns::DispatcherImpl::getClientConnection(info.iSessionId);
				if (ptrClient == nullptr)
				{
					return false;
				}
				else
				{
					iThreadId = ptrClient->getTId();
					type = ConnetionType::CT_CLIENT;
				}
			}
			else
			{
				iThreadId = ptrConnection->getTId();
				type = ConnetionType::CT_SERVER;
			}
			break;
		}
		case apie::ConnetionType::CT_SERVER:
		{
			auto ptrConnection = event_ns::DispatcherImpl::getConnection(info.iSessionId);
			if (ptrConnection == nullptr)
			{
				return false;
			}

			iThreadId = ptrConnection->getTId();
			break;
		}
		case apie::ConnetionType::CT_CLIENT:
		{
			auto ptrConnection = event_ns::DispatcherImpl::getClientConnection(info.iSessionId);
			if (ptrConnection == nullptr)
			{
				return false;
			}

			iThreadId = ptrConnection->getTId();
			break;
		}
		default:
			break;
		}

		auto ptrThread = CtxSingleton::get().getThreadById(iThreadId);
		if (ptrThread == nullptr)
		{
			return false;
		}

		ProtocolHead head;
		head.iSeqNum = info.iSeqNum;
		head.iFlags = info.getFlags();
		head.iOpcode = info.iOpcode;
		head.iBodyLen = (uint32_t)msg.size();

		if (info.getFlags() == 0)
		{
			SendData* itemObjPtr = new SendData;
			itemObjPtr->type = type;
			itemObjPtr->iSerialNum = info.iSessionId;
			itemObjPtr->sData.append(reinterpret_cast<char*>(&head), sizeof(ProtocolHead));
			itemObjPtr->sData.append(msg);

			Command command;
			command.type = Command::send_data;
			command.args.send_data.ptrData = itemObjPtr;
			ptrThread->push(command);
		}
		else
		{
			SendDataByFlag* itemObjPtr = new SendDataByFlag;
			itemObjPtr->type = type;
			itemObjPtr->iSerialNum = info.iSessionId;
			itemObjPtr->head = head;
			itemObjPtr->sBody = msg;

			Command command;
			command.type = Command::send_data_by_flag;
			command.args.send_data_by_flag.ptrData = itemObjPtr;
			ptrThread->push(command);
		}

		return true;
	}


	bool OutputStream::sendCommand(ConnetionType type, uint64_t iSerialNum, apie::Command& cmd)
	{
		uint32_t iThreadId = 0;

		switch (type)
		{
		case apie::ConnetionType::CT_SERVER:
		{
			auto ptrConnection = event_ns::DispatcherImpl::getConnection(iSerialNum);
			if (ptrConnection == nullptr)
			{
				return false;
			}

			iThreadId = ptrConnection->getTId();
			break;
		}
		case apie::ConnetionType::CT_CLIENT:
		{
			auto ptrConnection = event_ns::DispatcherImpl::getClientConnection(iSerialNum);
			if (ptrConnection == nullptr)
			{
				return false;
			}

			iThreadId = ptrConnection->getTId();
			break;
		}
		default:
			break;
		}

		auto ptrThread = CtxSingleton::get().getThreadById(iThreadId);
		if (ptrThread == nullptr)
		{
			return false;
		}

		ptrThread->push(cmd);

		return true;
	}

} // namespace Network
} // namespace Envoy
