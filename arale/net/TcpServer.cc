
#include <arale/base/Logging.h>
#include <arale/net/EventLoop.h>
#include <arale/net/Acceptor.h>
#include <arale/net/TcpServer.h>
#include <arale/net/SocketsOps.h>

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
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";
    for(auto it = connections_.begin(); it != connections_.end(); ++it) {
        TcpConnctionPtr conn = it->second;
        it->second.reset();
        EventLoop* ioLoop = conn->getLoop();
        ioLoop->runInLoop(std::bind(&TcpConnection::connectionDestroyed, conn));
        conn.reset();
    }
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

    InetAddress localaddr(sockets::getLocalAddr(newconnfd));
    TcpConnctionPtr newConn = std::make_shared<TcpConnection>(loop_, connName, newconnfd,
                                                                localaddr, peeraddr);
    newConn->setConnectionCallback(connectionCallback_);
    newConn->setMessageCallback(messageCallback_);
    newConn->setWriteCompleteCallback(writeCompleteCallback_);
    newConn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
    connections_[connName] = newConn;
    // here you can use shared_ptr object to replace a pointor to TcpConnection object
    loop_->runInLoop(std::bind(&TcpConnection::connectionEstablished, newConn));
}

void TcpServer::removeConnection(const TcpConnctionPtr &connection) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, connection));
}

void TcpServer::removeConnectionInLoop(const TcpConnctionPtr &conn) {
    loop_->assertInLoopThread();
    std::string connName = conn->getName();
    auto it = connections_.find(connName);
    assert(it != connections_.end());
    size_t ret = connections_.erase(connName);
    assert(ret == 1);
    (void)ret;
    EventLoop *ioLoop = conn->getLoop();
    
    // can NOT invoke runInLoop because we are in loop thread certainly,
    // we will continue to invoke TcpConnection::connectionDestroyed immediately,
    // that means when we finish the further invocation and get back to the 
    // function TcpConnection::handleClose, we still in event handle.
    // We will destruct TcpConneciton object then, which will cause Channel object
    // to be destructed, this disobey with the principle that do NOT destory Channel
    // object during a event handle
    //ioLoop->runInLoop(std::bind(&TcpConnection::connectionDestroyed, conn));
    
    // we post it so TcpConnection::connectionDestroyed will be called after 
    // the event handle loop
    // if a Channel object can be destory during event handle, we have to use this
    ioLoop->postFuntor(std::bind(&TcpConnection::connectionDestroyed, conn));
}

void TcpServer::start() {
    if (started_.getAndAdd(1) == 0) {
        assert(!accptor_->isListening());
        loop_->runInLoop(std::bind(&Acceptor::startListening, accptor_.get()));
    }
}

}

}
