
#include "Chargen.h"

#include <arale/base/Logging.h>
#include <arale/net/EventLoop.h>
#include <functional>
#include <stdio.h>

using namespace arale;
using namespace arale::net;

ChargenServer::ChargenServer(EventLoop *loop, const InetAddress &addr, bool print)
    : server_(loop, "Chargen Server", addr),
      transferred_(0),
      startTime_(Timestamp::now()) {
      using namespace std::placeholders;
    server_.setConnectionCallback(
      std::bind(&ChargenServer::onConnection, this, _1));
    server_.setMessageCallback(
      std::bind(&ChargenServer::onMessage, this, _1, _2, _3));
    server_.setWriteCompleteCallback(
      std::bind(&ChargenServer::onWriteComplete, this, _1));

    if (print) {
        loop->runEvery(3.0, std::bind(&ChargenServer::printThroughput, this));
    }

    std::string line;
    for(int i = 33; i < 127; ++i) {
        line.push_back(char(i));
    }
    line += line;
    for(size_t i = 0; i < 127 - 33; ++i) {
        message_ += line.substr(i, 72) + '\n';
    }
}

void ChargenServer::start() {
    server_.start();
}

void ChargenServer::onConnection(const TcpConnectionPtr & conn) {
    LOG_INFO << "Chargen Server - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");
    if (conn->isConnected()) {
        conn->setTcpNoDelay(true);
        conn->send(message_);
    }
}

void ChargenServer::onMessage(const TcpConnectionPtr &conn, Buffer * buf, Timestamp time) {
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->getName() << " discards " << msg.size()
             << " bytes received at " << time.toString();
}

void ChargenServer::onWriteComplete(const TcpConnectionPtr & conn) {
    transferred_ += message_.size();
    conn->send(message_);
}

void ChargenServer::printThroughput() {
    Timestamp endTime = Timestamp::now();
    double time = timeDifference(endTime, startTime_);
    printf("%4.3f MB/s\n", static_cast<double>(transferred_) / time / 1024 / 1024);
    transferred_ = 0;
    startTime_ = endTime;
}