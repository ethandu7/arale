
#ifndef ARALE_NET_POLLER_H
#define ARALE_NET_POLLER_H

#include <arale/net/EventLoop.h>

namespace arale {

namespace net {

class EventLoop;

class Poller {
public:
    Poller(EventLoop *loop);
    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    virtual ~Poller();
private:
    EventLoop *loop_;
};

}

}

#endif
