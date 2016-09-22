
#include "Discard.h"

#include <arale/base/Logging.h>

#include <functional>

using namespace arale;
using namespace arale::net;

DiscardServer::DiscardServer(EventLoop *loop, const InetAddress &serverAddr)
    : server_(loop, "DiscardServer", serverAddr) {
    using namespace std::placeholders;
    server_.setConnectionCallback(std::bind(&DiscardServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&DiscardServer::onMessage, this, _1, _2, _3));
}

void DiscardServer::start() {
    server_.start();
}

void DiscardServer::onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "DiscardServer - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");
}

void DiscardServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time) {
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->getName() << " discards " << msg.size()
             << " bytes received at " << time.toString();
    LOG_INFO << "discard content: " << msg;
}