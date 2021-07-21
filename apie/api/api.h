#pragma once

#include <memory>
#include <string>

#include "apie/common/time.h"
#include "apie/event/dispatcher.h"

namespace apie {
namespace api {

/**
 * "Public" API that different components use to interact with the various system abstractions.
 */
class Api {
public:
  virtual ~Api() {}

  /**
   * Allocate a dispatcher.
   * @return Event::DispatcherPtr which is owned by the caller.
   */
  virtual event_ns::DispatcherPtr allocateDispatcher(event_ns::EThreadType type, uint32_t tid) PURE;


};

typedef std::unique_ptr<Api> ApiPtr;

} // namespace Api
} // namespace Envoy
