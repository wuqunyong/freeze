#pragma once

#include <array>
#include <cstdint>
#include <string>

#include <event2/util.h>
#include <google/protobuf/message.h>

#include "apie/network/address.h"
#include "apie/network/windows_platform.h"
#include "apie/network/i_poll_events.hpp"
#include "apie/network/command.h"
#include "apie/proto/init.h"



namespace apie {
	class Command;

namespace network {

	class OutputStream
	{
	public:
		static bool sendMsg(uint64_t iSessionId, uint32_t iOpcode, const ::google::protobuf::Message& msg, ConnetionType type = ConnetionType::CT_NONE);
		static bool sendMsgByFlag(uint64_t iSessionId, uint32_t iOpcode, uint8_t iFlag, const ::google::protobuf::Message& msg, ConnetionType type = ConnetionType::CT_NONE);
		static bool sendMsgImpl(MessageInfo info, const ::google::protobuf::Message& msg);


		static bool sendMsgByStr(uint64_t iSerialNum, uint32_t iOpcode, const std::string& msg, ConnetionType type = ConnetionType::CT_NONE);
		static bool sendMsgByStrByFlag(uint64_t iSerialNum, uint32_t iOpcode, const std::string& msg, uint32_t iFlag, ConnetionType type = ConnetionType::CT_NONE);


		static bool sendCommand(ConnetionType type, uint64_t iSerialNum, apie::Command& cmd);
	};


}
}
