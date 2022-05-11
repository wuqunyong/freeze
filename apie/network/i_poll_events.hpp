#pragma once
 
#include "apie/common/pure.h"

namespace apie
{
 
	struct EndPoint
	{
		uint32_t realm = 0;
		uint32_t type = 0;
		uint32_t id = 0;
		std::string auth;

		EndPoint() = default;
		EndPoint(uint32_t realm, uint32_t type, uint32_t id, std::string auth)
		{
			this->realm = realm;
			this->type = type;
			this->id = id;
			this->auth = auth;
		}

		bool operator<(const EndPoint& other) const
		{
			if (realm != other.realm)
			{
				return realm < other.realm;
			}

			if (type != other.type)
			{
				return type < other.type;
			}

			if (id != other.id)
			{
				return id < other.id;
			}

			return false;
		}
	};

	enum class ConnetionType
	{
		CT_NONE = 0,
		CT_CLIENT = 1,
		CT_SERVER = 2,
	};

	enum class ProtocolType
	{
		PT_None = 0,
		PT_PB = 1,
		PT_HTTP = 2,
		PT_PBMsgHead = 3,
		PT_PBMsgUser = 4,
		PT_MAX,
	};

    // Virtual interface to be exposed by object that want to be notified
    // about events on file descriptors.
 
    struct i_poll_events
    {
        virtual ~i_poll_events () {}
 
        // Called by I/O thread when file descriptor is ready for reading.
        virtual void readcb() PURE;
 
        // Called by I/O thread when file descriptor is ready for writing.
        virtual void writecb() PURE;
 
        // Called when timer expires.
        virtual void eventcb(short what) PURE;
    };
 
}


