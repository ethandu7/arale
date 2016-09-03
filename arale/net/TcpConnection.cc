
#include <arale/base/Logging.h>
#include <arale/net/TcpConnection.h>
#include <arale/net/Socket.h>
#include <arale/net/Channel.h>
#include <arale/net/EventLoop.h>

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

void TcpConnection::connectionEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    // make sure we will not be destroyed when channel tries to handle us
    readWriteChannel_->tieTo(shared_from_this());
    readWriteChannel_->enableRead();
    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
}

}

}
