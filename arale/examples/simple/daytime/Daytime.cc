
#include "Daytime.h"

#include <arale/base/Logging.h>
#include <arale/net/EventLoop.h>
#include <functional>

using namespace arale;
using namespace arale::net;

DaytimeServer::DaytimeServer(EventLoop *loop, const InetAddress &listenAddr)
    : server_(loop, "Daytime Server", listenAddr) {
    using namespace std::placeholders;
    server_.setConnectionCallback(std::bind(&DaytimeServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&DaytimeServer::onMessage, this, _1, _2, _3));
}

void DaytimeServer::start() {
    server_.start();
}

void DaytimeServer::onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "DaytimeServer - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");

    if (conn->isConnected()) {
        conn->send(Timestamp::now().toFormattedString() + "\n");
        conn->shutdown();
    }
}

void DaytimeServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time) {
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->getName() << " discards " << msg.size()
             << " bytes received at " << time.toString();
}