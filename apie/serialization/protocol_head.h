#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>

#include "apie/serialization/byte_buffer.h"

// Last bit for an expanded message without compression.
const uint8_t PH_DEFAULT = 0b0u;

// Last bit for a compressed message.
const uint8_t PH_COMPRESSED = 0b1u;
const uint8_t PH_CRYPTO = 0b10u;

/*
Byte order:little-endian
Native byte order is big-endian or little-endian, depending on the host system. For example, Intel x86 and AMD64 (x86-64) are little-endian;
Byte
/       0       |       1       |       2       |       3       |
/               |               |               |               |
|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|
+---------------+---------------+---------------+---------------+
0|                           iSeqNum    
+---------------+---------------+---------------+---------------+
4| ..........R|C|   --iMagic--  |          ---iOpcode---                 
+---------------+---------------+---------------+---------------+
8|                           iBodyLen    
+---------------+---------------+---------------+---------------+
12|          iCheckSum          |              Data
+---------------+---------------+---------------+---------------+
16|                             Data
+---------------+---------------+---------------+---------------+
*/

#pragma pack(1)

struct ProtocolHead
{
	ProtocolHead(void)
	{
		this->iSeqNum = 0;
		this->iFlags = 0;
		this->iMagic = 0;
		this->iOpcode = 0;
		this->iBodyLen = 0;
		this->iCheckSum = 0;
	}

	uint32_t iSeqNum;
	uint8_t iFlags;
	uint8_t iMagic;
	uint16_t iOpcode;
	uint32_t iBodyLen;  
	uint32_t iCheckSum;
};

#pragma pack()

extern ByteBuffer& operator >> (ByteBuffer& stream, ProtocolHead& data);
extern ByteBuffer& operator << (ByteBuffer& stream, ProtocolHead data);


