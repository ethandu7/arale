
#ifndef ARALE_NET_TIMERID_H
#define ARALE_NET_TIMERID_H

namespace arale {

namespace net {

class Timer;

class TimerID {
public:
    TimerID() : timer_(nullptr), sequence_(0) {   }
    
    TimerID(Timer* timer, int64_t seq) :
        timer_(timer), sequence_(seq) 
    {    }

    friend class TimerQueue;

private:
    
    Timer* timer_;
    int64_t sequence_;

};

}

}

#endif
