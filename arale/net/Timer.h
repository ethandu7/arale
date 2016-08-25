
#ifndef ARALE_NET_TIMER_H
#define ARALE_NET_TIMER_H

#include <arale/base/Atomic.h>
#include <arale/base/TimeStamp.h>
#include <cstdint>
#include <functional>
//#include <cstdatomic>   //not the same as c++11 stand, it should be <atomic> according to the stand

namespace arale {

namespace net {
    
class Timer {
public:
    typedef std::function<void ()> TimerCallback; 
    
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
    const int64_t sequence_;

    static AtomicInt64 counter_;
};

}

}
#endif
