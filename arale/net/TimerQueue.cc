
// for the 'UINTPTR_MAX'
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

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

void TimerQueue::readTimerfd(const int timerfd, Timestamp now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
    if (n != sizeof howmany)
    {
      LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
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
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    
    // two lines below are not ture for sure, cause the timer may be added asynchronously
    // by another thread, the timer may not in the timerset yet, but it's canceled
    //assert(timers_.find(Entry(timer->timer_->getExpiration()), timer->timer_));
    //assert(activeTimers_.find(ActiveTimerEntry(timer->timer_, timer.sequence_)) 
    //    != activeTimers_.end());

    ActiveTimerEntry canceledTimer(timer.timer_, timer.sequence_);
    auto it = activeTimers_.find(canceledTimer);
    if (it != activeTimers_.end()) {
        size_t num = timers_.erase(Entry(it->first->getExpiration(), timer.timer_));
        assert(num == 1);
        num = activeTimers_.erase(canceledTimer);
        assert(num == 1);
        (void)num;
        // container's erase operating doesn't call delete on the inside objects if they are pointer
        // whereas it will call the deconstructor of the inside objects
        // if we don't want to use delete here, we can useing scoped_ptr instead of raw pointer
        delete it->first;
    } else if (isHandlingTimeout_) {
        // the isHandlingTimeout_ is for self-cancel, the timer handler wants to
        // cancel itself
        // when we get here, the timer is not in the timers_ nor in the activeTimers_
        // it's only in a temporary set, see the function getExpiredTimers
        // we need to remeber which timer is canceled by itself
        // then we will not handle this timer again, see code in function resetTimers
        std::pair<ActiveTimerSet::iterator, bool> result 
            = cancelledTimers_.insert(canceledTimer);
        assert(result.second);
        (void)result;
    }
    assert(timers_.size() == activeTimers_.size());
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

std::vector<TimerQueue::Entry> TimerQueue::getExpiredTimers(Timestamp now) {
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expiredTimers;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    // it will point to the timer after the last timeout timer
    auto it = timers_.lower_bound(sentry);
    // at this moment, the vector is empty, we need insert_iterator
    // don't know whether we can use the moves for Entry objects??? 
    std::copy(timers_.begin(), it, back_inserter(expiredTimers));

    timers_.erase(timers_.begin(), it);
    for(auto it1 = expiredTimers.begin(); it1 != expiredTimers.end(); ++it1) {
        ActiveTimerEntry entry(it1->second, it1->second->getSequence());
        size_t num = activeTimers_.erase(entry);
        assert(num == 1);
        (void)num;
    }
    assert(timers_.size() == activeTimers_.size());
    // I think RVO will be triggered
    return expiredTimers;
}

void TimerQueue::resetTimers(const std::vector<Entry> &timers, Timestamp now) {
    assert(timers_.size() == activeTimers_.size());

    for(auto it = timers.begin(); it != timers.end(); ++it) {
        Timer *timer = it->second;
        ActiveTimerEntry entry(timer, timer->getSequence());
        if (timer->isRepeated() && cancelledTimers_.find(entry) == cancelledTimers_.end()) {
            timer->reset(now);
            insert(timer);
        } else {
            delete it->second;
        }
    }
    
    if (!timers_.empty()) {
        Timestamp newExpiration = timers_.begin()->second->getExpiration();
        if (newExpiration.valid()) {
            resetTimerfd(timerfd_, newExpiration);
        }
    }
    
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleTimeout() {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    Timestamp now = Timestamp::now();
    // have to read the timerfd once it expires to clear the readable flag
    readTimerfd(timerfd_, now);

    std::vector<Entry> expiredTimers = getExpiredTimers(now);
    isHandlingTimeout_ = true;
    cancelledTimers_.clear();
    for (auto it = expiredTimers.begin(); it != expiredTimers.end(); ++it) {
        it->second->run();
    }
    isHandlingTimeout_ = false;
    resetTimers(expiredTimers, now);
    assert(timers_.size() == activeTimers_.size());
}

}

}
