
#ifndef ARALE_NET_TIMERQUEUE_H
#define ARALE_NET_TIMERQUEUE_H

#include <arale/base/TimeStamp.h>
#include <arale/net/Channel.h>


#include <vector>
#include <set>

namespace arale {

namespace net {

class EventLoop;
class Timer;
class TimerID;

class TimerQueue {
public:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerSet;
    
    TimerQueue(EventLoop *loop);
    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;
    ~TimerQueue();

    void handleTimeout();

    static int createTimerFd();
private:
    EventLoop *loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerSet timers_;
};

}

}

#endif
