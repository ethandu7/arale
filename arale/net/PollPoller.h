
#ifndef ARALE_NET_POLLPOLLER_H
#define ARALE_NET_POLLPOLLER_H

#include <arale/net/Poller.h>

// this must be here, in the global space, don't know why
struct pollfd;

namespace arale {

namespace net {

class EventLoop;

class PollPoller : public Poller {
public:
    PollPoller(EventLoop* loop);

    virtual ~PollPoller();
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
    virtual void insertChannel(Channel *channel);
    virtual void removeChannel(Channel *channel);

private:
    void fillActiveChannels(int numEvents, ChannelList *activeChannels);
    using PollfdList = std::vector<struct pollfd>;
    PollfdList pollfds_;
};

}

}

#endif
