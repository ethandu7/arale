
#ifndef ARALE_NET_TIMERQUEUE_H
#define ARALE_NET_TIMERQUEUE_H

#include <arale/base/TimeStamp.h>
#include <arale/net/Channel.h>
#include <arale/net/Callbacks.h>


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
    typedef std::pair<Timer*, int64_t> ActiveTimerEntry;
    typedef std::set<ActiveTimerEntry>   ActiveTimerSet;
    
    TimerQueue(EventLoop *loop);
    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;
    ~TimerQueue();

    TimerID addTimer(const TimerCallback &timerCallback, Timestamp when, double interval);
    void cancelTimer(TimerID timer);
 
private:
    void addTimerInLoop(Timer* timer);
    void cancelTimerInLoop(TimerID timer);
    
    static int createTimerFd();
    void handleTimeout();
    std::vector<Entry> getExpiredTimers(Timestamp now);
    bool insert(Timer* timer);
    void resetTimers(const std::vector<Entry> &timers, Timestamp now);
    void readTimerfd(const int timerfd, Timestamp now);
    void resetTimerfd(const int timerfd, Timestamp when);
    
    EventLoop *loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerSet timers_;

    // for cancel functionality
    ActiveTimerSet activeTimers_;
    bool isInHandlingTimeout_;
    ActiveTimerSet cancelledTimers_;
};

}

}

#endif
