
#include <arale/net/Poller.h>
#include <arale/net/PollPoller.h>

namespace arale {

namespace net {

Poller* Poller::setDefaultPoller(EventLoop * loop) {
    return new PollPoller(loop);
}

}

}
