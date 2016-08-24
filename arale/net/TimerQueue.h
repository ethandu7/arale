
#ifndef ARALE_NET_TIMERQUEUE_H
#define ARALE_NET_TIMERQUEUE_H

#include <arale/net/Channel.h>

#include <vector>

namespace arale {

namespace net {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue {
public:
    TimerQueue(EventLoop *loop);
    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

    ~TimerQueue();

private:
    EventLoop *loop_;
    Channel timerfdChannel_;
};

}

}

#endif
