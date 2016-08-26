
#include <arale/base/CurrentThread.h>
#include <arale/base/TimeStamp.h>
#include <arale/net/EventLoop.h>
#include <arale/net/Poller.h>
#include <arale/net/TimerQueue.h>
#include <arale/net/Channel.h>

#include <sys/eventfd.h>

using namespace arale::net;

// only available in this file
namespace {

__thread EventLoop* currentThreadEventLoop;

const int kPollTimeMs = 10000;

int creatWeakupFd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}
    
}

namespace arale {

namespace net {

EventLoop::EventLoop() :
    threadID_(base::getCurrentThreadID()),
    islooping_(false),
    poller_(Poller::setDefaultPoller(this)), // is it safe?
    timerQueue_(new TimerQueue(this)), // is this safe?
    isHandlingPendingFunctors_(false),
    quit_(false),
    wakeupFd_(creatWeakupFd()),
    wakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG_TRACE << "EventLoop is created " << this << " in thread " << threadID_;
    if (currentThreadEventLoop) {
        LOG_FATAL << "Another EventLoop " << currentThreadEventLoop
              << " exists in this thread " << threadID_;
    } else {
        currentThreadEventLoop = this;
    }

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleWakeup, this));
    wakeupChannel_->enableRead();
}

EventLoop::~EventLoop() {
    assert(islooping_== false);
    LOG_DEBUG << "EventLoop " << this << " of thread " << threadID_
            << " destructs in thread " << base::getCurrentThreadID();
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    currentThreadEventLoop = NULL;
}

void EventLoop::loop() {
    assert(islooping_== false);
    assertInLoopThread();
    islooping_ = true;
    quit_ = false;
    LOG_TRACE << "EventLoop " << this << " start looping";

    while (!quit_) {
        activeChannels_.clear();
        Timestamp pollReturnTime = poller_->poll(kPollTimeMs, &activeChannels_);
        for(ChannelList::const_iterator it = activeChannels_.begin(); 
            it != activeChannels_.end(); ++it) {
            (*it)->handleEvent(pollReturnTime);
        }
        // since we handle pending functors here, we don't have to wake up 
        // loop thread if we post pending functors in event callback
        // because the post happened before this
        // but if we post pending functors in handling pending functors
        // we need to do wake up when we do the post, or those new added functors
        // will get done after next poll
        doPendingFunctors();
    }
    
    LOG_TRACE << "EventLoop " << this << " stop looping";    
    islooping_ = false;
}

EventLoop* getCurrentEventLoop() {
    return currentThreadEventLoop;
}

void EventLoop::updateChannel(Channel * channel) {
    assert(channel->getEventLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel * channel) {
    assert(channel->getEventLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

void EventLoop::runInLoop(const InLoopFunctor &functor) {
    if (isInLoopThread()) {
        functor();
    } else {
        postFuntor(functor);
    }
}

void EventLoop::postFuntor(const InLoopFunctor & functor) {
    // reduce the critical area
    {
        std::lock_guard<std::mutex> guard(pendingMutex_);
        pendingFunctors_.push_back(functor);
    }

    // if we are not in loop thread,
    // or we are in loop thread and we are also handling pending functors
    // then we need to weak up(again) the loop thread
    // so those new added pending functors can be done ASAP
    //
    // we don't weak up loop thread if we are posting functors in event callback
    //
    // this weak up policy is based on the place function "doPendingFunctors" is called
    if (!isInLoopThread() || isHandlingPendingFunctors_) {
        weakup();
    }
}

void EventLoop::doPendingFunctors() {
    assertInLoopThread();
    isHandlingPendingFunctors_ = true;
    PendingFunctorList functors;

    // using local vector to store the functors
    // so we can reduce the critical area, may have perfermance loss
    {
        std::lock_guard<std::mutex> guard(pendingMutex_);
        functors.swap(pendingFunctors_);
    }

    for(size_t i  = 0; i < functors.size(); ++i) {
        functors[i]();
    }

    isHandlingPendingFunctors_ = false;
}

void EventLoop::handleWakeup() {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::weakup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

TimerID EventLoop::runAt(Timestamp when, const TimerCallback & callback) {
    return timerQueue_->addTimer(callback, when, 0.0);
}

TimerID EventLoop::runAfter(double delay, const TimerCallback & callback) {
    Timestamp when(addTime(Timestamp::now(), delay));
    return timerQueue_->addTimer(callback, when, 0.0);
}

TimerID EventLoop::runEvery(double interval, const TimerCallback & callback) {
    Timestamp when(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(callback, when, interval);
}

void EventLoop::quitLoop() {
    quit_ = true;
    // loop thread may wait on the poller
    // wake up it so the loop can end ASAP
    if (!isInLoopThread()) {
        weakup();
    }
}

}

}
