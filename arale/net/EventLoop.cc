
#include <arale/net/EventLoop.h>
#include <arale/base/CurrentThread.h>

using namespace arale;
using namespace arale::net;

EventLoop::EventLoop() :
    threadID_(base::getCurrentThreadID())
{

}
