
#include "Echo.h"

#include <arale/net/EventLoop.h>

#include <functional>

using namespace arale;
using namespace arale::net;
using namespace arale::base;


EchoServer::EchoServer(EventLoop *loop, const InetAddress &addr, int maxConn)
    : server_(loop, "Echo Server", addr),
      numConnection_(0),
      maxConnections_(maxConn) {
    using namespace std::placeholders;
    server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start() {
    server_.start();
}

void EchoServer::onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "Echo Server - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");

    if (conn->isConnected()) {
        ++numConnection_;
        if (numConnection_ > maxConnections_) {
            conn->shutdown();
            conn->forceCloseWithDelay(3.0);
        }
    } else {
        --numConnection_;
    }
    LOG_INFO << "numConnected = " << numConnection_;
}

void EchoServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->getName() << " echo " << msg.size()
             << " bytes received at " << receiveTime.toString();
    conn->send(msg);
}