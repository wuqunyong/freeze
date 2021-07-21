#pragma once

#include "apie/event/timer.h"
#include "apie/common/utility.h"

namespace apie {
namespace event_ns {

/**
 * Real-world time implementation of TimeSystem.
 */
class RealTimeSystem : public TimeSystem {
public:
  // TimeSystem
  SchedulerPtr createScheduler(Scheduler&) override;

  // TimeSource
  SystemTime systemTime() override { return time_source_.systemTime(); }
  MonotonicTime monotonicTime() override { return time_source_.monotonicTime(); }

private:
	common::RealTimeSource time_source_;
};

} // namespace Event
} // namespace Envoy
