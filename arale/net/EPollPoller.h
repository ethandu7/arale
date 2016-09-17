
#ifndef ARALE_NET_EPOLLPOLLER_H
#define ARALE_NET_EPOLLPOLLER_H

#include <arale/net/Poller.h>

#include <vector>

struct epoll_event;

namespace arale {

namespace net {

class EventLoop;

class EPollPoller : public Poller {
public:
    EPollPoller(EventLoop *);
    virtual ~EPollPoller();
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels);
    virtual void updateChannel(Channel *channel);
    virtual void removeChannel(Channel *channel);
private:
    typedef std::vector<struct epoll_event> EPollEventList;
    static const int kInitEPollEventListSize = 16;
    void fillActiveChannels(int, ChannelList *);
    void update(int operation, Channel *ch);

    int  epollfd_;
    EPollEventList epollEvents_;
};

}

}

#endif
