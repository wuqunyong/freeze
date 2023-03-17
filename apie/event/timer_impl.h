#pragma once

#include <chrono>
#include <map>
#include <atomic>

#include "apie/event/timer.h"

#include "apie/event/event_impl_base.h"
#include "apie/event/libevent.h"
#include "apie/singleton/threadsafe_singleton.h"

namespace apie {
namespace event_ns {

/**
 * libevent implementation of Timer.
 */
class TimerImpl : public Timer, ImplBase {
public:
	TimerImpl(libevent::BasePtr& libevent, TimerCb cb);

	// Timer
	void disableTimer() override;
	void enableTimer(const std::chrono::milliseconds& d) override;
	bool enabled() override;

private:
	TimerCb cb_;

public:
	static std::atomic<uint32_t> s_callCount;
};


class EphemeralTimer
{
public:
	void enableTimer(uint64_t interval);

private:
	uint64_t m_id = 0;
	event_ns::TimerPtr m_timer;

	friend class EphemeralTimerMgr;
};

class EphemeralTimerMgr
{
public:
	std::shared_ptr<EphemeralTimer> createEphemeralTimer(TimerCb cb);
	void deleteEphemeralTimer(uint64_t id);

public:
	uint64_t m_id = 0;
	std::map<uint64_t, std::shared_ptr<EphemeralTimer>> m_ephemeralCache;
};

using EphemeralTimerMgrSingleton = ThreadSafeSingleton<EphemeralTimerMgr>;

} // namespace Event
} // namespace Envoy
