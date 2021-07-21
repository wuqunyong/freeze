#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "apie/common/time.h"
#include "apie/event/file_event.h"
#include "apie/event/signal.h"
#include "apie/event/timer.h"
#include "apie/event/deferred_deletable.h"

#include "apie/network/listener.h"
#include "apie/network/command.h"


namespace apie {
namespace event_ns {

	enum class EThreadType
	{
		TT_None = 0,
		TT_Listen,
		TT_IO,
		TT_Logic,
		TT_DB,
		TT_Log,
		TT_Metrics,
		TT_Nats,
	};

	inline std::string toStirng(EThreadType type)
	{
		switch (type)
		{
		case apie::event_ns::EThreadType::TT_Listen:
		{
			return "TT_Listen";
		}
		case apie::event_ns::EThreadType::TT_IO:
		{
			return "TT_IO";
		}
		case apie::event_ns::EThreadType::TT_Logic:
		{
			return "TT_Logic";
		}
		case apie::event_ns::EThreadType::TT_Log:
		{
			return "TT_Log";
		}
		case apie::event_ns::EThreadType::TT_Metrics:
		{
			return "TT_Metrics";
		}
		default:
			break;
		}

		return "None";
	}

/**
 * Callback invoked when a dispatcher post() runs.
 */
typedef std::function<void()> PostCb;

/**
 * Abstract event dispatching loop.
 */
class Dispatcher {
public:
  virtual ~Dispatcher() {}


  virtual void clearDeferredDeleteList() PURE;

  virtual network::ListenerPtr createListener(network::ListenerCbPtr cb, network::ListenerConfig config) PURE;

  virtual event_ns::TimerPtr createTimer(TimerCb cb) PURE;

  virtual void deferredDelete(DeferredDeletablePtr&& to_delete) PURE;

  virtual void start() PURE;

  virtual void exit() PURE;

  virtual SignalEventPtr listenForSignal(int signal_num, SignalCb cb) PURE;

  virtual void post(PostCb callback) PURE;

  virtual void run(void) PURE;

  virtual void push(Command& cmd) PURE;

  virtual std::atomic<bool>& terminating() PURE;
};

typedef std::unique_ptr<Dispatcher> DispatcherPtr;

} // namespace Event
} // namespace APie
