#pragma once

#include "../network/listener.h"
#include "../event/dispatcher_impl.h"

namespace apie {
namespace network {

/**
 * libevent implementation of Network::Listener for TCP.
 */
class ListenerImpl : public Listener {
public:
  ListenerImpl(event_ns::DispatcherImpl& dispatcher, ListenerCbPtr cb, network::ListenerConfig config);
  ~ListenerImpl();

  void disable() override;
  void enable() override;

protected:
  void setupServerSocket(event_ns::DispatcherImpl& dispatcher);

  event_ns::DispatcherImpl& dispatcher_;
  ListenerCbPtr cb_;
  network::ListenerConfig config_;

private:
  static void listenCallback(evconnlistener*, evutil_socket_t fd, sockaddr* remote_addr, int remote_addr_len, void* arg);
  static void errorCallback(evconnlistener* listener, void* context);

  struct sockaddr_in listener_add_;
  event_ns::Libevent::ListenerPtr listener_;
};

} // namespace Network
} // namespace Envoy
