
#include "TimeClient.h"

#include <arale/net/EventLoop.h>
#include <stdio.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

int main(int argc, char *argv[]) {
    LOG_INFO << "pid = " << getCurrentThreadID();
    if (argc > 1)
    {
        EventLoop loop;
        InetAddress serverAddr(argv[1], 2016);
        TimeClient timeClient(&loop, serverAddr);
        timeClient.connect();
        loop.loop();
    }
    else
    {
        printf("Usage: %s host_ip\n", argv[0]);
    }
}
