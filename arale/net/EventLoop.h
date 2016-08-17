
#ifndef ARALE_NET_EVENTLOOP_H
#define ARALE_NET_EVENTLOOP_H

#include <sys/types.h>



namespace arale {

namespace net {

class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    
private:
    const pid_t threadID_;

};

};

};


#endif

