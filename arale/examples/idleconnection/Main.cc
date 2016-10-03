
#include "Echo.h"

#include <arale/net/EventLoop.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

int main(int argc, char *argv[]) {
    EventLoop loop;
    InetAddress addr(2016);
    int idleSeconds = 10;
    if (argc > 1) {
        idleSeconds = atoi(argv[1]);
    }
    LOG_INFO << "pid = " << getCurrentThreadID()
             << ", idle seconds = " << idleSeconds;
    EchoServer server(&loop, addr, idleSeconds);
    server.start();
    loop.loop();
}

