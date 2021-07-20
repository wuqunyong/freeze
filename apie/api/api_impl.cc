#include "../api/api_impl.h"

#include <chrono>
#include <string>

#include "../event/dispatcher_impl.h"

namespace apie {
namespace api {

Impl::Impl()
{
}

event_ns::DispatcherPtr Impl::allocateDispatcher(event_ns::EThreadType type, uint32_t tid) {
  return std::make_unique<event_ns::DispatcherImpl>(type, tid);
}


} // namespace Api
} // namespace Envoy
