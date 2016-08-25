#include <arale/base/Types.h>
#include <arale/net/Channel.h>
#include <arale/net/PollPoller.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>

namespace arale {

namespace net {

PollPoller::PollPoller(EventLoop * loop) :
    Poller(loop)
{

}

PollPoller::~PollPoller() {

}

// don't do event dispatch here, that is Channel object's job, let EventLoop trigger it
// also if we handle event here, which may make the pollfds_ changes its size, it's dangerous 
Timestamp PollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    // should have &* in the front, vector<struct pollfd>::iterator can not be converted to pollfd * 
    //int nEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    // in c++11 it can be pollfds_data()
    int nEvents = ::poll(pollfds_.data(), pollfds_.size(), timeoutMs);
    int savedErrno = errno;
    Timestamp returnTime(Timestamp::now());
    if (nEvents > 0) {
        LOG_TRACE << nEvents << "events happened";
        fillActiveChannels(nEvents, activeChannels);
    } else if (nEvents == 0) {
        LOG_TRACE << "nothing happened";
    } else {
        // not signal occurred before any requested event
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_SYSERR << "error happened at poll";
        }
    }
    return returnTime;
}

void PollPoller::fillActiveChannels(int numEvents, ChannelList * activeChannels) {
    // cannot use range for, which is simple
    // for (struct pollfd pollfd : pollfds_)
    for (PollfdList::const_iterator pfd = pollfds_.begin(); 
            pfd != pollfds_.end() && numEvents; ++pfd) {
        if (pfd->revents > 0) {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel =  ch->second;
            assert(channel->getfd() == pfd->fd);
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}


// this will be called by EventLoop
void PollPoller::updateChannel(Channel * channel) {
    Poller::assertInLoopThread();
    LOG_TRACE << "fd = " << channel->getfd() << " events = " << channel->getEvents();
    //  new added channel
    if (channel->getIndex() < 0) {
        assert(channels_.find(channel->getfd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->getfd();
        pfd.events = static_cast<short>(channel->getEvents());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        channel->setIndex(static_cast<int>(pollfds_.size() - 1));
        channels_[pfd.fd] = channel;
    } else {
        // updating an existing one
        assert(channels_.find(channel->getfd()) != channels_.end());
        // it should be a reference, not an object
        struct pollfd &pfd = pollfds_[channel->getIndex()];
        assert(pfd.fd == channel->getfd() || pfd.fd == -channel->getfd() - 1);
        pfd.events = static_cast<short>(channel->getEvents());
        pfd.revents = 0;
        // when up level call disableAll()
        if (channel->isNoneEvent()) {
            // let poller ignor this pollfd
            // note: the fd also is the key in the map, we can not change the key
            //       so we have inconsistent fds at two different places
            pfd.fd = -channel->getfd() - 1;
        }
    }
}

void PollPoller::removeChannel(Channel * channel) {
    Poller::assertInLoopThread();
    LOG_TRACE << "fd = " << channel->getfd();
    assert(channels_.find(channel->getfd()) != channels_.end());
    int index = channel->getIndex();
    assert(index >= 0 && index < static_cast<int>(pollfds_.size()));
    // a channel must have no events on it before it can be removed from the poller's fd set
    // that means the fd in this channel is reset
    assert(channel->isNoneEvent());
    size_t n  = channels_.erase(channel->getfd());
    assert(n == 1);
    (void)n;
    
    if (static_cast<size_t>(index) != pollfds_.size() - 1) {
        int lastChannel = pollfds_.back().fd;
        struct pollfd &pfd = pollfds_[index];
        // explicitly inhibit the ADL and any custom swap method
        std::swap(pfd, pollfds_.back());
        if (lastChannel < 0) {
            lastChannel = -lastChannel - 1;
        }
        channels_[lastChannel]->setIndex(index);
    }
    pollfds_.pop_back();
}

}

}
