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

Timestamp PollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    // should have &* in the front, vector<struct pollfd>::iterator can not be converted to pollfd* 
    int nEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
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

void PollPoller::insertChannel(Channel * channel) {

}

void PollPoller::removeChannel(Channel * channel) {

}

}

}
