
#ifndef ARALE_NET_EVENTLOOP_H
#define ARALE_NET_EVENTLOOP_H

#include <arale/base/CurrentThread.h>
#include <arale/base/Logging.h>
#include <arale/net/TimerID.h>
#include <arale/net/Callbacks.h>

#include <memory>   // for smart pointers
#include <vector>
#include <functional>

namespace arale {

namespace net {

class Poller;
class TimerQueue;
class Channel;

class EventLoop {
public:
    typedef std::function<void ()> InLoopFunctor;
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

    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);

    void runInLoop(const InLoopFunctor functor);

    static EventLoop* getCurrentEventLoop();

    void quit() { quit_ = true; }

    // timer interface
    TimerID runAt(Timestamp when, const TimerCallback& callback);
    TimerID runAfter(double delay, const TimerCallback& callback);
    TimerID runEvery(double interval, const TimerCallback& callback);
    
private:
    const pid_t threadID_;
    bool islooping_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    
    typedef std::vector<Channel*> ChannelList;
    ChannelList activeChannels_;

    bool quit_;
};

}

}

#endif

