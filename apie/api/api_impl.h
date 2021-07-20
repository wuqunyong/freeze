#pragma once

#include <chrono>
#include <string>

#include "../api/api.h"
#include "../event/timer.h"

namespace apie {
namespace api {

/**
 * Implementation of Api::Api
 */
class Impl : public Api {
public:
  Impl();

  // Api::Api
  event_ns::DispatcherPtr allocateDispatcher(event_ns::EThreadType type, uint32_t tid) override;
};

} // namespace Api
} // namespace Envoy
