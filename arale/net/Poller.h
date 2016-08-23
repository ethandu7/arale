
#ifndef ARALE_NET_POLLER_H
#define ARALE_NET_POLLER_H

#include <vector>
#include <map>
#include <arale/base/TimeStamp.h>
#include <arale/net/EventLoop.h>

namespace arale {

namespace net {

class Channel;

class Poller {
public:
    using ChannelList = std::vector<Channel *>;
    
    Poller(EventLoop *loop);
    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    virtual ~Poller();
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void insertChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    void assertInLoopThread() {
        loop_->assertInLoopThread();
    }
    
protected:
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap channels_;

private:    
    EventLoop *loop_;
};

}

}

#endif
