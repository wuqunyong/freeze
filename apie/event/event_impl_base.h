#pragma once

#include "event2/event_struct.h"

namespace apie {
namespace event_ns {

/**
 * Base class for libevent event implementations. The event struct is embedded inside of this class
 * and derived classes are expected to assign it inside of the constructor.
 */
class ImplBase {
protected:
	~ImplBase();

	event raw_event_;
};

} // namespace Event
} // namespace Envoy
