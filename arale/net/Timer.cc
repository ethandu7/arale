
#include <arale/net/Timer.h>

namespace arale {

namespace net {

AtomicInt64 Timer::counter_;

Timer::Timer(const TimerCallback &timerCallback, Timestamp expiration, double interval) :
    timerCallback_(timerCallback),
    expiration_(expiration),
    interval_(interval),
    sequence_(counter_.incrementAndGet())
{

}

void Timer::reset(Timestamp time) {
    if(interval_ > 0.0) {
        expiration_ = addTime(time, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}

}

}
