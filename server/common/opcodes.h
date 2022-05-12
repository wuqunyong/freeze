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
		//ҵ������룬��1000��ʼ
		OP_MSG_REQUEST_ACCOUNT_LOGIN_L = 1000,
		OP_MSG_RESPONSE_ACCOUNT_LOGIN_L = 1001,

		OP_MSG_REQUEST_CLIENT_LOGIN = 1002,
		OP_MSG_RESPONSE_CLIENT_LOGIN = 1003,

		OP_MSG_REQUEST_ECHO = 1004,
		OP_MSG_RESPONSE_ECHO = 1005,

		OP_MSG_REQUEST_HANDSHAKE_INIT = 1006,
		OP_MSG_RESPONSE_HANDSHAKE_INIT = 1007,
		OP_MSG_REQUEST_HANDSHAKE_ESTABLISHED = 1008,
		OP_MSG_RESPONSE_HANDSHAKE_ESTABLISHED = 1009,


		_MSG_CLIENT_LOGINTOL = 501,
		_MSG_CLIENT_LOGINTOG = 502,
		_MSG_GAMESERVER_LOGINRESP = 504,
		_MSG_MAP_USER_CMD = 701,
		_MSG_USER_INFO = 720,
		_MSG_TALENT_CMD = 750,

	};



	enum ReturnCode : uint32_t
	{
		RC_OK = 0,

		//ҵ�񷵻��룬��1000��ʼ
	};
}
