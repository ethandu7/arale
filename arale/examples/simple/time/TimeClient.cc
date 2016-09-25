
#include "TimeClient.h"

#include <arale/base/Logging.h>
#include <arale/net/EventLoop.h>

#include <functional>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

TimeClient::TimeClient(EventLoop *loop, const InetAddress &addr)
    : loop_(loop),
      client_(loop, "Time Client", addr) {
      using namespace std::placeholders;
    client_.setConnectionCallback(std::bind(&TimeClient::onConnection, this, _1));
    client_.setMessageCallback(std::bind(&TimeClient::onMessage, this, _1, _2, _3));
}

void TimeClient::connect() {
    client_.connect();
}

void TimeClient::onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "TimeClient - " << conn->localAddr().toIpPort() << " -> "
                 << conn->remoteAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");
    if (!conn->isConnected()) {
        loop_->quitLoop();
    }
}

void TimeClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
    if (buf->readableBytes() >= sizeof(uint32_t)) {
        const void *data = buf->peek();
        uint32_t be32 = *static_cast<const uint32_t *>(data);
        buf->retrieve(sizeof(uint32_t));
        time_t time = sockets::networkToHost32(be32);
        Timestamp ts(implicit_cast<uint64_t>(time) * Timestamp::kMicroSecondsPerSecond);
        LOG_INFO << "Server time = " << time << ", " << ts.toFormattedString();
    } else {
        LOG_INFO << conn->getName() << " no enough data " << buf->readableBytes()
                 << " at " << receiveTime.toFormattedString();
    }
}