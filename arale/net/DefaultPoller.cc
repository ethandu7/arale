
#include <arale/net/Poller.h>
#include <arale/net/PollPoller.h>
#include <arale/net/EPollPoller.h>

#include <stdlib.h>

namespace arale {

namespace net {

Poller* Poller::setDefaultPoller(EventLoop * loop) {
    if (getenv("ARALE_USE_POLL")) {
        return new PollPoller(loop);
    } else {
        return new EPollPoller(loop);
    }
}

}

}
