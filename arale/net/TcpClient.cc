
#include <arale/base/Logging.h>
#include <arale/net/TcpClient.h>
#include <arale/net/EventLoop.h>
#include <arale/net/Connector.h>
#include <arale/net/SocketsOps.h>
#include <arale/net/TcpConnection.h>

#include <functional>
#include <stdio.h>

namespace arale {

namespace net {

using namespace std::placeholders;

TcpClient::TcpClient(EventLoop *loop, const string &name, const InetAddress &serverAddr)
    : loop_(loop),
      name_(name),
      connID_(1),
      connector_(new Connector(loop, serverAddr)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback) {
      connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, _1));
      LOG_INFO << "TcpClient::TcpClient[" << name_
               << "] - connector " << connector_.get();
}

TcpClient::~TcpClient() {

}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toIpPort().c_str(), connID_);
    ++connID_;
    std::string connName = name_ + buf;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnctionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    conn->setMessageCallback(messageCallback_);
    conn->setConnectionCallback(connectionCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    // don't know why
    {
        std::lock_guard<std::mutex> guard(mutex_);
        connector_ = conn;
    }
    loop_->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn));
}

}

}
