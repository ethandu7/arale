
#include <arale/base/Logging.h>
#include <arale/net/TcpConnection.h>
#include <arale/net/Socket.h>
#include <arale/net/Channel.h>
#include <arale/net/EventLoop.h>
#include <arale/net/SocketsOps.h>

#include <functional>

namespace arale {

namespace net {

using namespace std::placeholders;

void defaultConnectionCallback(const TcpConnctionPtr &conn) {
    LOG_TRACE << conn->localAddr().toIpPort() << " -> "
              << conn->remoteAddr().toIpPort() << " is "
              << (conn->isConnected() ? "UP" : "DOWN");
}

void defaultMessageCallback(const TcpConnctionPtr & conn, Buffer *buf, Timestamp receiveTime) {
    //buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop *loop, const string &name, int sockfd,
        const InetAddress &localaddr, const InetAddress &remoteaddr) 
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      readWriteChannel_(new Channel(loop_, sockfd)),
      localAddr_(localaddr),
      remoteAddr_(remoteaddr) {
    readWriteChannel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
    readWriteChannel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    readWriteChannel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    readWriteChannel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this
            << " fd=" << sockfd;
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    // a TcpConnection object can be destructed in a non io thread
    //loop_->assertInLoopThread();
    LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
            << " fd=" << readWriteChannel_->getfd()
            << " state=" << stateToString();
    assert(state_ == kDisconnected);
}

void TcpConnection::connectionEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    // make sure we will not be destroyed when channel tries to handle us
    readWriteChannel_->tieTo(shared_from_this());
    readWriteChannel_->enableRead();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectionDestroyed() {
    loop_->assertInLoopThread();
    if (state_ == kConnected) {
        setState(kDisconnected);
        // duplicated with the invocation in handleClose 
        // the normal calling sequence is handClose first, then connectionDestroyed
        // but in some special case we can get here without calling handleClose
        // we have to add this
        readWriteChannel_->disableAll();
        // print out current state of this tcp connection
        connectionCallback_(shared_from_this());
    }
    readWriteChannel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    loop_->assertInLoopThread();
    
}

void TcpConnection::handleWrite() {

}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    // duplicated with the invocation in connectionDestroyed 
    readWriteChannel_->disableAll();
    TcpConnctionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}

void TcpConnection::handleError() {
    loop_->assertInLoopThread();
    int err = sockets::getSocketError(socket_->getSockfd());
    LOG_ERROR << "TcpConnection::handleError [" << name_
              << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

const char* TcpConnection::stateToString() const {
    switch (state_) {
        case kDisconnected:
            return "kDisconnected";
        case kConnected:
            return "kConnected";
        case kConnecting:
            return "kConnecting";
        case kDisconnecting:
            return "kDisconnecting";
    }
}

}

}
