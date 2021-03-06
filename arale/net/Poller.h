
#ifndef ARALE_NET_POLLER_H
#define ARALE_NET_POLLER_H

#include <vector>
#include <map>
#include <arale/base/TimeStamp.h>
#include <arale/net/EventLoop.h>

namespace arale {

namespace net {

class Channel;


// this class doesn't own Channel objects
// every Channel object should register itself to poller after construction 
// and unregister itself from poller before destruction
class Poller {
public:
    // g++ 4.4.6 doesn't know this type alias, only know typedef
    //using ChannelList = std::vector<Channel *>;
    typedef std::vector<Channel *> ChannelList;
    
    Poller(EventLoop *loop);
    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    virtual ~Poller();
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    void assertInLoopThread() {
        loop_->assertInLoopThread();
    }
    static Poller* setDefaultPoller(EventLoop *loop);
protected:
    //using ChannelMap = std::map<int, Channel*>;
    typedef std::map<int, Channel*> ChannelMap;

    // store the poniter to Channel that need to be polled
    // key is the fd associated with the Channel
    ChannelMap channels_;

private:    
    EventLoop *loop_;
};

}

}

#endif
