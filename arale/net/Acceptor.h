
#ifndef ARALE_NET_ACCEPTOR_H
#define ARALE_NET_ACCEPTER_H

#include <arale/net/Channel.h>
#include <arale/net/Socket.h>

#include <functional>


namespace arale {

namespace net {

class InetAddress;
class EventLoop;

class Acceptor {
public:
    typedef std::function<void (int sockFd, const InetAddress&)>  AcceptorCallback;
    Acceptor(EventLoop* loop);
    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

private:
    const listenFd_;
    EventLoop *loop_;
    Channel accceptChannel_;
    Socket acceptSockte_;
    bool isListenning_
};

}

}

#endif
