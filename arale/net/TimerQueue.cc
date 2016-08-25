
#include <arale/base/Logging.h>
#include <arale/net/TimerQueue.h>
#include <arale/net/EventLoop.h>
#include <arale/net/TimerID.h>
#include <arale/net/Timer.h>

#include <functional>
#include <sys/timerfd.h>

using namespace arale;

namespace {

struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100)
  {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(
      microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

}
namespace arale {

namespace net {

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

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    close(timerfd_);
}

void TimerQueue::resetTimerfd(const int timerfd, Timestamp when) {
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(when);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        LOG_SYSERR << "timerfd_settime()";
    }

}

TimerID TimerQueue::addTimer(const TimerCallback & timerCallback, 
    Timestamp when, double interval) 
{
    Timer *timer = new Timer(timerCallback, when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerID(timer, timer->getSequence());
}

void TimerQueue::addTimerInLoop(Timer *timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    
    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->getExpiration());
    }
}

void TimerQueue::cancelTimer(TimerID timer) {
    loop_->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop, this, timer));
}

void TimerQueue::cancelTimerInLoop(TimerID timer) {

}

bool TimerQueue::insert(Timer * timer) {
    assert(timers_.size() == activeTimers_.size());
    loop_->assertInLoopThread();
    Timestamp when = timer->getExpiration();
    auto it = timers_.begin();
    
    bool needResetTimerfd = false;
    if (it == timers_.end() || when < it->first) {
        needResetTimerfd = true;
    }

    {
        std::pair<TimerSet::iterator, bool> result 
            = timers_.insert(Entry(when, timer));
        assert(result.second);
        (void)result;
    }

    {
        std::pair<ActiveTimerSet::iterator, bool> result
            = activeTimers_.insert(ActiveTimerEntry(timer, timer->getSequence()));
        assert(result.second);
        (void)result;
    }
    assert(timers_.size() == activeTimers_.size());
    
    return needResetTimerfd;
}

void TimerQueue::handleTimeout() {
    
}

}

}