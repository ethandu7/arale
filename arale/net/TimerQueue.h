
#ifndef ARALE_NET_TIMERQUEUE_H
#define ARALE_NET_TIMERQUEUE_H

namespace arale {

namespace net {

class EventLoop;

class TimerQueue {
public:
    TimerQueue(EventLoop *loop);
    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

    ~TimerQueue();

private:
    EventLoop *loop_;

};

}

}

#endif
