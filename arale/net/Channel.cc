
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
    isInLoop_(false),
    index_(-1),
    tied_(false),
    isHandlingEvent_(false)
{
    
}

Channel::~Channel() {
    assert(!isInLoop_);
    assert(!isHandlingEvent_);
}

void Channel::update() {
    isInLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove() {
    isInLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::tieTo(const  std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::handleEvent(Timestamp receiveTime) {
    // make sure when do the callback, the object callback is runing on is still there
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        if (guard) {
            handleEventWithGuard(receiveTime);
        }
    } else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
    isHandlingEvent_ = true;
    
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

    isHandlingEvent_ = false;
}

}

}


