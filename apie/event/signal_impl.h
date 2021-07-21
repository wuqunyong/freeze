#pragma once

#include "apie/event/signal.h"

#include "apie/event/dispatcher_impl.h"
#include "apie/event/event_impl_base.h"

namespace apie {
namespace event_ns {

/**
 * libevent implementation of Event::SignalEvent.
 */
class SignalEventImpl : public SignalEvent, ImplBase {
public:
  SignalEventImpl(DispatcherImpl& dispatcher, int signal_num, SignalCb cb);

private:
  SignalCb cb_;
};

} // namespace Event
} // namespace Envoy
