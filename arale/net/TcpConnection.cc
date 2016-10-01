
#include <arale/base/Logging.h>
#include <arale/base/WeakCallback.h>

#include <arale/net/TcpConnection.h>
#include <arale/net/Socket.h>
#include <arale/net/Channel.h>
#include <arale/net/EventLoop.h>
#include <arale/net/SocketsOps.h>

#include <functional>

namespace arale {

namespace net {

using namespace std::placeholders;

void defaultConnectionCallback(const TcpConnectionPtr &conn) {
    LOG_TRACE << conn->localAddr().toIpPort() << " -> "
              << conn->remoteAddr().toIpPort() << " is "
              << (conn->isConnected() ? "UP" : "DOWN");
}

void defaultMessageCallback(const TcpConnectionPtr & conn, Buffer *buf, Timestamp receiveTime) {
    buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop *loop, const string &name, int sockfd,
        const InetAddress &localaddr, const InetAddress &remoteaddr) 
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      readWriteChannel_(new Channel(loop_, sockfd)),
      localAddr_(localaddr),
      remoteAddr_(remoteaddr),
      highWaterMark_(64*1024*1024),
      reading_(false) {
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

bool TcpConnection::getTcpInfo(struct tcp_info *tcpi) const {
    return socket_->getTcpInfo(tcpi);
}

std::string TcpConnection::getTcpInfoString() const {
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof(buf));
    return buf;
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
    int saveErrno = 0;
    size_t res = inputBuffer_.readFd(socket_->getSockfd(), &saveErrno);
    if (res > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (res == 0) {
        handleClose();
    } else {
        errno = saveErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
    
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    // I think this is just for test
    // it must be writing in channel
    if (readWriteChannel_->isWriting()) {
        ssize_t res = sockets::write(socket_->getSockfd(), 
            outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (res > 0) {
            outputBuffer_.retrieve(res);
            if (outputBuffer_.readableBytes() == 0) {
                readWriteChannel_->disableWrite();
                if (writeCompleteCallback_) {
                    // why can't we invoke the writeCompleteCallback_ here directly
                    // in a event handle callback
                    loop_->postFuntor(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            } 
        } else {
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
    } else {
        LOG_TRACE << "Connection fd = " << readWriteChannel_->getfd()
              << " is down, no more writing";
    }   
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    // duplicated with the invocation in connectionDestroyed 
    readWriteChannel_->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}

void TcpConnection::handleError() {
    loop_->assertInLoopThread();
    int err = sockets::getSocketError(socket_->getSockfd());
    LOG_ERROR << "TcpConnection::handleError [" << name_
              << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        // if we cann't shutdown this time
        // the next line tells the handlewrite function to shutdwon 
        // when all the data have been sent out
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    // in some case we may still have data to write
    // we can't shutdown write until all the data have been sent out
    if (!readWriteChannel_->isWriting()) {
        socket_->shutDownWrite();
    }
}

void TcpConnection::send(const void *data, size_t len) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(data, len);
        } else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, data, len));
        }
    }
}

void TcpConnection::send(const std::string &msg) {
    send(msg.c_str(), msg.size());
}

void TcpConnection::send(Buffer * buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        } else {
            // the evaluation of the last argument will happen at this bind call time
            loop_->runInLoop(std::bind(static_cast<void (TcpConnection::*)(const std::string &)>(&TcpConnection::send), 
                                this, buf->retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
    loop_->assertInLoopThread();
    size_t left = len;
    ssize_t nwrote = 0;
    bool error = false;
    // if no data is queuing in the buffer
    // try to send out directly
    if (!readWriteChannel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = sockets::write(socket_->getSockfd(), data, len);
        if (nwrote >= 0) {
            left = len - nwrote;
            if (left == 0 && writeCompleteCallback_) {
                loop_->postFuntor(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_SYSERR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    error= true;
                }
            }
        }
    } 

    assert(left <= len);
    if (!error && left > 0) {
        size_t having = outputBuffer_.readableBytes();
        // highwatermark handling
        if (having < highWaterMark_ &&
            having + left >= highWaterMark_ &&
            highWaterMarkCallback_) {
            loop_->postFuntor(std::bind(highWaterMarkCallback_, shared_from_this(), having + left));
        }
        outputBuffer_.append(data, left);
        if (!readWriteChannel_->isWriting()) {
            readWriteChannel_->enableWrite();
        }
    }
}

void TcpConnection::forceClose() {
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::forceCloseInLoop, this));
    }
}

void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    assert(state_ == kDisconnecting);
    handleClose();
}

void TcpConnection::forceCloseWithDelay(double seconds) {
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
        loop_->runAfter(seconds, makeWekaCallback(shared_from_this(), 
            &TcpConnection::forceClose));
    }
}

void TcpConnection::startRead() {
    assert(state_ == kConnected);
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop() {
    loop_->assertInLoopThread();
    if (!reading_ || !readWriteChannel_->isReading()) {
        readWriteChannel_->enableRead();
        reading_ = true;
    }
}

void TcpConnection::stopRead() {
    assert(state_ ==kConnected);
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop() {
    loop_->assertInLoopThread();
    if (reading_ || readWriteChannel_->isReading()) {
        readWriteChannel_->disableRead();
        reading_ = false;
    }
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
        default:
            return "unknown state";
    }
}

}

}
