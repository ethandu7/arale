

#include <arale/base/CurrentThread.h>
#include <arale/net/EventLoop.h>
#include <arale/net/Poller.h>
#include <arale/net/TimerQueue.h>

using namespace arale;
using namespace arale::net;

EventLoop::EventLoop() :
    threadID_(base::getCurrentThreadID()),
    poller_(),
    timerQueue_()
{

}

void EventLoop::loop() {
    islooping = true;
}
