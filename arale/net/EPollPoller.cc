
#include <arale/base/Logging.h>
#include <arale/net/EPollPoller.h>
#include <arale/net/EventLoop.h>
#include <arale/net/Channel.h>

#include <sys/epoll.h>
#include <assert.h>
#include <errno.h>

namespace {

// a channel is not in poller's channel set
const int kNew = -1;
// the fd is handled by epoll
const int kAdded = 1;
// if a channel is with this flag, the associated fd is not handled by epoll
// but the channel is still in poller's channel set
const int kDeleted = 2;

}

namespace arale {

namespace net {

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop),
      epollfd_(epoll_create1(EPOLL_CLOEXEC)),
      epollEvents_(kInitEPollEventListSize){
    if (epollfd_ < 0) {
        LOG_SYSFATAL << "EPollPoller::EPollPoller";
    }
}

EPollPoller::~EPollPoller() {
    close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    LOG_TRACE << "fd total count " << channels_.size();
    int num = epoll_wait(epollfd_, epollEvents_.data(), 
        static_cast<int>(epollEvents_.size()), timeoutMs);
    Timestamp now(Timestamp::now());
    int savedErrno = errno;
    if (num > 0) {
        fillActiveChannels(num, activeChannels);
        if (implicit_cast<size_t>(num) == epollEvents_.size()) {
            epollEvents_.resize(epollEvents_.size() * 2);
        }
    } else if (num == 0) {
        LOG_TRACE << "nothing happended";
    } else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_SYSERR << "EPollPoller::poll()";
        }
    }
    return now;
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activChannelList) {
    assert(implicit_cast<size_t>(numEvents) <= epollEvents_.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel *ch = static_cast<Channel *>(epollEvents_[i].data.ptr);
#ifndef NDEBUG
        int fd = ch->getfd();
        auto it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == ch);
#endif
        ch->set_revents(epollEvents_[i].events);
        activChannelList->push_back(ch);
    }
}

void EPollPoller::updateChannel(Channel *channel) {
    Poller::assertInLoopThread();
    const int index = channel->getIndex();
    int fd = channel->getfd();
    LOG_TRACE << "fd = " << channel->getfd()
              << " events = " << channel->getEvents() << " index = " << index;
    // add new fd to epoll
    if (index == kNew || index == kDeleted) {       
        assert(channels_.find(fd) == channels_.end());
        if (index == kNew) {
            channels_[fd] = channel;
        } else {
            assert(channels_[fd] == channel);
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // update existinng fd in epoll
        // or remove fd from epoll
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            channel->setIndex(kDeleted);
            update(EPOLL_CTL_DEL, channel);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel) {
    Poller::assertInLoopThread();
    int fd = channel->getfd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->getIndex();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);
    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

void EPollPoller::update(int operation, Channel *ch) {
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = ch->getEvents();
    event.data.ptr = ch;
    int fd = ch->getfd();
    LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
              << " fd = " << fd << " event = { " << ch->eventsToString() << " }";
    int ret = epoll_ctl(epollfd_, operation, fd, &event);
    if (ret < 0) {
        if (operation == kDeleted) {
            LOG_SYSERR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        }
    }
}

const char* EPollPoller::operationToString(int operation) {
    switch (operation) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR operation");
            return "Unknown Operation";
    }
}

}

}
