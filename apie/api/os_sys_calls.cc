#include "apie/api/os_sys_calls.h"

#include <chrono>
#include <string>

#include "apie/network/windows_platform.h"

namespace apie {
namespace api {

uint32_t OsSysCalls::getCurProcessId()
{
#ifdef WIN32
	uint32_t pid = (uint32_t)GetCurrentProcessId();
	return pid;
#else
	uint32_t pid = (uint32_t)getpid();
	return pid;
#endif
}

} // namespace Api
} // namespace APie
