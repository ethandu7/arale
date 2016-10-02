
#include "Echo.h"

#include <arale/net/EventLoop.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

int main() {
    LOG_INFO << "pid = " << getCurrentThreadID();
    EventLoop loop;
    InetAddress addr(2016);
    int maxConnections = 5;
    LOG_INFO << "maxConnections = " << maxConnections;
    EchoServer server(&loop, addr, maxConnections);
    server.start();
    loop.loop();
}

