
#include <arale/base/Logging.h>
#include <arale/net/TcpClient.h>
#include <arale/net/EventLoop.h>
#include <arale/net/Connector.h>
#include <arale/net/SocketsOps.h>

#include <functional>
#include <stdio.h>

using namespace arale::net;
namespace {

void removeTcpConnection(EventLoop* loop, const TcpConnectionPtr& conn) {
    loop->postFuntor(std::bind(&TcpConnection::connectionDestroyed, conn));
}

}

namespace arale {

namespace net {

using namespace std::placeholders;

TcpClient::TcpClient(EventLoop *loop, const string &name, const InetAddress &serverAddr)
    : loop_(loop),
      name_(name),
      connID_(1),
      retry_(false),
      connect_(true),
      connector_(new Connector(loop, serverAddr)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback) {
      connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, _1));
      LOG_INFO << "TcpClient::TcpClient[" << name_
               << "] - connector " << connector_.get();
}

TcpClient::~TcpClient() {
    LOG_INFO << "TcpClient::~TcpClient[" << name_
             << "] - connector " << connector_.get();
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if (conn) {
        assert(loop_ == conn->getLoop());
        CloseCallback callback = std::bind(removeTcpConnection, loop_, _1);
        // we got make sure any operation on TcpConnetion object happens in io thread
        // why do we make indirect call two times, why can't we just indirectily call
        // TcpConnection::connectionDestroyed ???
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, callback));
        // if we are the last one to hold this connection
        if (unique) {
            conn->forceClose();
        }
    } else {
        connector_->stop();
    }
}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toIpPort().c_str(), connID_);
    ++connID_;
    std::string connName = name_ + buf;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn(std::make_shared<TcpConnection>(loop_, connName, sockfd, localAddr, peerAddr));
    conn->setMessageCallback(messageCallback_);
    conn->setConnectionCallback(connectionCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, _1));
    {
        std::lock_guard<std::mutex> guard(mutex_);
        connection_ = conn;
    }
    loop_->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn));
}

void TcpClient::removeConnection(const TcpConnectionPtr &connection) {
    loop_->assertInLoopThread();
    assert(loop_ == connection->getLoop());
    assert(connection_ == connection);
    {
        std::lock_guard<std::mutex> guard(mutex_);
        connection_.reset();
    }
    loop_->postFuntor(std::bind(&TcpConnection::connectionDestroyed, connection));
    // if user shutdown explicitly, we don't retry
    if (retry_ && connect_) {
        LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
                 << connector_->getServerAddress().toIpPort();
        connector_->restart();
    }
}

void TcpClient::connect() {
    LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
             << connector_->getServerAddress().toIpPort();
    connect_ = true;
    // don't worry about the run this in io thread
    // the connector will take care of this
    // give the user a chance to call this funciton not in io thread
    connector_->start();
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TcpClient::disconnect() {
    connect_ = false;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (connection_) {
            connection_->shutdown();
        }
    }
}

}

}
