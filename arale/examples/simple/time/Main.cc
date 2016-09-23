
#include "Time.h"
#include <arale/net/EventLoop.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

int main() {
    LOG_INFO << "pid = " << getCurrentThreadID();
    EventLoop loop;
    InetAddress addr(2016);
    TimeServer server(&loop, addr);
    server.start();
    loop.loop();
}

