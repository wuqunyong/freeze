#include "apie/serialization/protocol_head.h"


ByteBuffer& operator >> (ByteBuffer& stream, ProtocolHead& data)
{
	stream >> data.iFlags;
	stream >> data.iMagic;
	stream >> data.iOpcode;
	stream >> data.iBodyLen;
	stream >> data.iCheckSum;

	return stream;
}

ByteBuffer& operator << (ByteBuffer& stream, ProtocolHead data)
{
	stream << data.iFlags;
	stream << data.iMagic;
	stream << data.iOpcode;
	stream << data.iBodyLen;
	stream << data.iCheckSum;

	return stream;
}

uint32_t MergeOpcode(uint16_t iType, uint16_t iCmd)
{
	uint32_t iOpcode = iType << 16;
	iOpcode = iOpcode + iCmd;
	return iOpcode;
}

std::tuple<uint16_t, uint16_t> SplitOpcode(uint32_t iOpcode)
{
	uint16_t iType = iOpcode >> 16;
	uint32_t iBase = (1 << 16);
	uint16_t iCmd = iOpcode % iBase;
	return std::make_tuple(iType, iCmd);
}