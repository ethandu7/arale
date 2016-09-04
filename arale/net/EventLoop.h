
#ifndef ARALE_NET_EVENTLOOP_H
#define ARALE_NET_EVENTLOOP_H

#include <arale/base/CurrentThread.h>
#include <arale/base/Logging.h>
#include <arale/net/TimerID.h>
#include <arale/net/Callbacks.h>

#include <memory>   // for smart pointers
#include <vector>
#include <functional>
#include <mutex>

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

    // if another object(such as TimerQueue) in other thread(non io thread) 
    // wants to put something into loop or remove something from loop, 
    // it should call this runInLoop function on loop object
    //
    // other object should define two different versions of one function
    // see TimerQueue::addTimer and TimerQueue::addTimerInLoop
    // the first verion uses the seconed version as a argument to call this runInLoop
    // and the second verison do the real adding or remove
    // by this way we can make sure we don't have to acquire a lock for adding or remove 
    void runInLoop(const InLoopFunctor &functor);    
    void postFuntor(const InLoopFunctor& functor);

    static EventLoop* getCurrentEventLoop();

    void quitLoop();

    // timer interface
    TimerID runAt(Timestamp when, const TimerCallback& callback);
    TimerID runAfter(double delay, const TimerCallback& callback);
    TimerID runEvery(double interval, const TimerCallback& callback);
    void cancelTimer(TimerID timer);
    
private:
    void weakup();
    void handleWakeup();
    void doPendingFunctors();
    
    const pid_t threadID_;
    bool islooping_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    
    typedef std::vector<Channel*> ChannelList;
    ChannelList activeChannels_;

    // for functor post subsystem
    std::mutex pendingMutex_;
    // we donn't want to weak up the loop thread if the loop is run those functors
    // because the loop is awake
    bool isHandlingPendingFunctors_;
    typedef std::vector<InLoopFunctor> PendingFunctorList;
    PendingFunctorList pendingFunctors_;

    bool quit_;

    // for weakup 
    const int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
};

}

}

#endif

