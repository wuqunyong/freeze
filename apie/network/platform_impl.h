#pragma once

#include <assert.h>

#include "apie/network/windows_platform.h"


namespace apie {

class PlatformImpl {
public:
  PlatformImpl() {
#ifdef WIN32
    const WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    const int rc = ::WSAStartup(wVersionRequested, &wsaData);
    assert(rc == 0);
#endif
  }

  ~PlatformImpl() { 
#ifdef WIN32
	  ::WSACleanup(); 
#endif
  }
};

} // namespace Envoy
