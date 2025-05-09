#include "apie/event/libevent.h"

#include <signal.h>

#include "event2/thread.h"

namespace apie {
namespace event_ns {
namespace libevent {

bool Global::initialized_ = false;

void Global::initialize() {
#ifdef WIN32
  evthread_use_windows_threads();
#else
  evthread_use_pthreads();

  // Ignore SIGPIPE and allow errors to propagate through error codes.
  signal(SIGPIPE, SIG_IGN);
#endif
  initialized_ = true;
}

} // namespace Libevent
} // namespace Event
} // namespace Envoy
