#pragma once

#ifdef WIN32
#else
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <memory>
#include <string>

#include "apie/singleton/threadsafe_singleton.h"

namespace apie {
namespace api {

class OsSysCalls {

public:
	uint32_t getCurProcessId();
};

typedef ThreadSafeSingleton<OsSysCalls> OsSysCallsSingleton;

} // namespace Api
} // namespace APie
