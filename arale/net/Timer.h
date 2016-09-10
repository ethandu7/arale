
#ifndef ARALE_NET_TIMER_H
#define ARALE_NET_TIMER_H

#include <arale/base/Atomic.h>
#include <arale/base/TimeStamp.h>
#include <arale/net/Callbacks.h>
#include <cstdint>
#include <functional>
//#include <cstdatomic>   //not the same as c++11 stand, it should be <atomic> according to the stand

namespace arale {

namespace net {
    
class Timer {
public:    
    Timer(const TimerCallback &timerCallback, Timestamp expiration, double interval);
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void reset(Timestamp time);

    void run() { timerCallback_(); }
    Timestamp getExpiration() { return expiration_; }
    int64_t getSequence()   { return sequence_; }
    bool isRepeated()   { return interval_ > 0.0; }
    static int64_t getCounter() { return counter_.get(); }
    
private:
    const TimerCallback timerCallback_;
    Timestamp expiration_;
    const double interval_;
    // in TimerQueue we use timer's address as key to sort timers
    // there may have a chance that two timers have the same address but one
    // is crated after another one which has the same address is already deleted
    // so we add a sequence to every timer, the counter is unique to every timer
    const int64_t sequence_;

    static AtomicInt64 counter_;
};

}

}
#endif
