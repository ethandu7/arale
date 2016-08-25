
#include <arale/base/Logging.h>
#include <arale/net/TimerQueue.h>
#include <arale/net/EventLoop.h>

#include <functional>
#include <sys/timerfd.h>


using namespace arale;

using namespace arale::net;

int TimerQueue::createTimerFd() {
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd < 0)
        LOG_SYSFATAL << "crate timerfd failed";
    return fd;
}

TimerQueue::TimerQueue(EventLoop* loop) :
    loop_(loop),
    timerfd_(createTimerFd()),
    timerfdChannel_(loop_, timerfd_)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleTimeout, this));
    timerfdChannel_.enableRead();
}

void TimerQueue::handleTimeout() {

}

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
}