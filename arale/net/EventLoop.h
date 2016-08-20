
#ifndef ARALE_NET_EVENTLOOP_H
#define ARALE_NET_EVENTLOOP_H

#include <arale/base/CurrentThread.h>
#include <arale/base/Logging.h>

#include <memory>   // c++ for smart pointers


namespace arale {

namespace net {

class Poller;
class TimerQueue;
class Channel;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    bool isInLoopThread() {
        return threadID_ == base::getCurrentThreadID();
    }
    void loop();

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
                      << " was created in threadId_ = " << threadID_
                      << ", current thread id = " <<  base::getCurrentThreadID();
        }
    }
private:
    const pid_t threadID_;
    bool islooping;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
};

}

}


#endif

