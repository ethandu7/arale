

#include <arale/base/CurrentThread.h>
#include <arale/net/EventLoop.h>
#include <arale/net/Poller.h>
#include <arale/net/TimerQueue.h>

//using namespace arale;
using namespace arale::net;


// only available in this file
namespace {

__thread EventLoop* currentThreadEventLoop;

}

namespace arale {

namespace net {

EventLoop::EventLoop() :
    threadID_(base::getCurrentThreadID()),
    islooping(false),
    poller_(),
    timerQueue_()
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
    assert(islooping == false);
    currentThreadEventLoop = NULL;
}

void EventLoop::loop() {
    assert(islooping == false);
    assertInLoopThread();
    islooping = true;
}

EventLoop* getCurrentEventLoop() {
    return currentThreadEventLoop;
}

}

}
