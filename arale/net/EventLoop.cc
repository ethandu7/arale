
#include <arale/base/CurrentThread.h>
#include <arale/base/TimeStamp.h>
#include <arale/net/EventLoop.h>
#include <arale/net/Poller.h>
#include <arale/net/TimerQueue.h>
#include <arale/net/Channel.h>

//using namespace arale;
using namespace arale::net;


// only available in this file
namespace {

__thread EventLoop* currentThreadEventLoop;

    const int kPollTimeMs = 10000;
}

namespace arale {

namespace net {

EventLoop::EventLoop() :
    threadID_(base::getCurrentThreadID()),
    islooping_(false),
    poller_(Poller::setDefaultPoller(this)), // is it safe?
    timerQueue_(),
    quit_(false)
{
    LOG_TRACE << "EventLoop is created " << this << " in thread " << threadID_;
    if (currentThreadEventLoop) {
        LOG_FATAL << "Another EventLoop " << currentThreadEventLoop
              << " exists in this thread " << threadID_;
    } else {
        currentThreadEventLoop = this;
    }
}

EventLoop::~EventLoop() {
    assert(islooping_== false);
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
        for(ChannelList::const_iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
            (*it)->handleEvent(pollReturnTime);
        }
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

void EventLoop::runInLoop(const InLoopFunctor functor) {
    if (isInLoopThread()) {
        functor();
    } else {

    }
}

}

}
