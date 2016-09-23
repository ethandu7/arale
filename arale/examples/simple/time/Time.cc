
#include "Time.h"

#include <arale/base/Logging.h>
#include <arale/net/Endian.h>

#include <functional>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

TimeServer::TimeServer(EventLoop *loop, const InetAddress &addr)
    : server_(loop, "Time Server", addr) {
    using namespace std::placeholders;
    server_.setConnectionCallback(std::bind(&TimeServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&TimeServer::onMessage, this, _1, _2, _3));
}

void TimeServer::start() {
    server_.start();
}

void TimeServer::onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "TimeServer - " << conn->remoteAddr().toIpPort() << " -> "
                 << conn->localAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");
    if (conn->isConnected()) {
        time_t now = time(NULL);
        uint32_t be32 = sockets::hostToNetwork32(static_cast<uint32_t>(now));
        conn->send(&be32, sizeof(be32));
        conn->shutdown();
    }
}

void TimeServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time) {
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->getName() << " discards " << msg.size()
             << " bytes received at " << time.toString();
}