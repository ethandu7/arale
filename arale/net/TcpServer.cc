
#include <arale/base/Logging.h>
#include <arale/net/EventLoop.h>
#include <arale/net/Acceptor.h>
#include <arale/net/TcpServer.h>

namespace arale {

namespace net {

using namespace std::placeholders;

TcpServer::TcpServer(EventLoop* loop, 
                    const std::string &name, 
                    const InetAddress &serveraddr,
                    Option option)
    : loop_(loop),
      name_(name),
      ipPort_(serveraddr.toIpPort()),
      nextConnId_(1),
      accptor_(new Acceptor(loop, serveraddr, option == kReusePort)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback) {
    accptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConncetion, this, _1, _2));
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
}

void TcpServer::newConncetion(int newconnfd, const InetAddress &peeraddr) {
    loop_->assertInLoopThread();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_
             << "] - new connection [" << connName
             << "] from " << peeraddr.toIpPort();

    TcpConnctionPtr newConn = std::make_shared<TcpConnection>(loop_, connName, newconnfd);
    newConn->setConnectionCallback(connectionCallback_);
    newConn->setMessageCallback(messageCallback_);
    newConn->setWriteCompleteCallback(writeCompleteCallback_);
    connections_[connName] = newConn;
}

}

}
