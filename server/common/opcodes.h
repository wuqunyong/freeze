#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"


namespace apie {

	enum Opcodes : uint16_t
	{
		//业务操作码，从1000开始

		OP_ClientLoginRequest = 1002,
		OP_ClientLoginResponse = 1003,

		OP_EchoRequest = 1004,
		OP_EchoResponse = 1005,

		OP_HandshakeInitRequest = 1006,
		OP_HandshakeInitResponse = 1007,
		OP_HandshakeEstablishedRequest = 1008,
		OP_HandshakeEstablishedResponse = 1009,

	};



	enum ReturnCode : uint32_t
	{
		RC_OK = 0,

		//业务返回码，从1000开始
	};

}
