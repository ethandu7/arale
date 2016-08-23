
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
    virtual void updateChannel(Channel *channel);
    virtual void removeChannel(Channel *channel);

private:
    void fillActiveChannels(int numEvents, ChannelList *activeChannels);
    // not support by  g++ 4.4.6
    //using PollfdList = std::vector<struct pollfd>;
    typedef std::vector<struct pollfd> PollfdList;
    PollfdList pollfds_;
};

}

}

#endif
