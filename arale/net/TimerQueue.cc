
#include <arale/net/TimerQueue.h>
//#include <arale/net/EventLoop.h>


using namespace arale;

using namespace arale::net;

TimerQueue::TimerQueue(EventLoop* loop) :
    loop_(loop)
{

}

TimerQueue::~TimerQueue() {

}