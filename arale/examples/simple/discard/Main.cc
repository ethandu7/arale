
#include "Discard.h"


#include <arale/base/Logging.h>
#include <arale/net/EventLoop.h>

using namespace arale;
using namespace arale::base;
using namespace arale::net;

int main() {
    LOG_INFO << "pid = " << getCurrentThreadID();
    EventLoop loop;
    InetAddress listenAddr(2016);
    DiscardServer discardSever(&loop, listenAddr);
    discardSever.start();
    loop.loop();
}
