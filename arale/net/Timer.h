
#ifndef ARALE_NET_TIMER_H
#define ARALE_NET_TIMER_H

#include <functional>

namespace arale {

namespace net {
class Timer {
public:
    typedef std::function<void ()> TimerCallback; 
    
    Timer(TimerCallback timerCallback);
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    
private:
    const TimerCallback timerCallback_;
};

}

}
#endif
