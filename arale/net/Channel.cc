
#include <arale/base/Logging.h>
#include <arale/net/Channel.h>
#include <arale/net/EventLoop.h>
#include <poll.h>


namespace arale {

namespace net {

// don't forget const qualifier
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop * loop, int socketFD) :
    loop_(loop), 
    fd_(socketFD), 
    events_(kNoneEvent),
    revents_(kNoneEvent),
    isInLoop(false)
{
    
}

Channel::~Channel() {
    assert(!isInLoop);
}

void Channel::update() {
    isInLoop = true;
    loop_->updateChannel(this);
}

void Channel::remove() {
    isInLoop = false;
    loop_->removeChannel(this);
}

void Channel::HandleEvent(Timestamp receiveTime) {

    // try to write but peer already close the channel
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        LOG_WARN << "POLLHUP is happening";
        if (closeCallback_)
            closeCallback_();
    }

    // create new socket failed
    if (revents_ & POLLNVAL ) {
        LOG_WARN << "POLLNVAL is happening";
    }

    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_)
            errorCallback_();
    }

    // got some date
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_)
            readCallback_(receiveTime);
    }

    // wrote some data
    if (revents_ & POLLOUT) {
        if (writeCallback_)
            writeCallback_();
    }
}

}

}


