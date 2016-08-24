
#include <arale/net/EventLoop.h>
#include <arale/net/Channel.h>
#include <sys/timerfd.h>
#include <stdio.h>
#include <strings.h>

using namespace arale::net;

EventLoop* g_loop;

void timeout(arale::Timestamp, Channel *channel) {
    printf("Timeout!\n");
    //g_loop->quit();
    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(channel->getfd(), 0, &howlong, NULL);
}

int main() {

    EventLoop loop;
    g_loop = &loop;

    using namespace std::placeholders;
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(g_loop, timerfd);
    channel.setReadCallback(std::bind(timeout, _1, &channel));
    channel.enableRead();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);

    loop.loop();

    ::close(timerfd);
}