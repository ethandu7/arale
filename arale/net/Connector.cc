
#include <arale/base/Logging.h>
#include <arale/net/EventLoop.h>
#include <arale/net/SocketsOps.h>
#include <arale/net/Channel.h>
#include <arale/net/Connector.h>

#include <errno.h>

namespace arale {

namespace net {

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connected_(false),
      retryDelayMs_(kInitRetryDelayMs) {

}

Connector::~Connector() {

}

void Connector::start() {
    connected_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
    loop_->assertInLoopThread();
    if (connected_) {
        connect();
    } else {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::connect() {
    int sock = sockets::createNonblockingOrDie(serverAddr_.family());
    int ret = sockets::connect(sock, serverAddr_.getSockAddr());
    int savedError = (ret == 0 ? 0 : errno);
    switch (savedError) {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sock);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sock);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG_SYSERR << "connect error in Connector::startInLoop " << savedError;
            sockets::close(sock);
            break;

        default:
            LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedError;
            sockets::close(sock);
            break;
    }
}

void Connector::connecting(int sockfd) {
    loop_->assertInLoopThread();
    assert(connected_);
    assert(!connectChannel_);
    connectChannel_.reset(new Channel(loop_, sockfd));
    // when socket is writeable, the connection is establised
    connectChannel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
    connectChannel_->setErrorCallback(std::bind(&Connector::handleError, this));
    connectChannel_->enableWrite();
}

// the connection is establised
void Connector::handleWrite() {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    if (err) {
        LOG_WARN << "Connector::handleWrite - SO_ERROR = "
               << err << " " << strerror_tl(err);
        retry(sockfd);
    } else if (sockets::isSelfConnect(sockfd)) {
        LOG_WARN << "Connector::handleWrite - Self connect";
        retry(sockfd);
    } else {
        if (connected_) {
            connectionCallback_(sockfd);
        } else {
            sockets::close(sockfd);
        }
    }
}

int Connector::removeAndResetChannel() {
    connectChannel_->disableAll();
    connectChannel_->remove();
    int sock = connectChannel_->getfd();
    // we cannot reset channel here, cause we are in the channel callback
    // the reset will destruct channel
    loop_->postFuntor(std::bind(&Connector::resetChannel, this));
    return sock;
}

void Connector::resetChannel() {
    connectChannel_.reset();
}

// precondition: the channel associated with the socket must be removed from poller 
void Connector::retry(int sockfd) {
    sockets::close(sockfd);
    if (connected_) {
        LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
                 << " in " << retryDelayMs_<< " milliseconds. ";
        // make sure after few seconds the connector is still there
        loop_->runAfter(retryDelayMs_, std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    } else {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::handleError() {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
    retry(sockfd);
}

void Connector::stop() {
    connected_ = false;
    loop_->runInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop() {
    loop_->assertInLoopThread();
    int sockfd = removeAndResetChannel();
    retry(sockfd);
}

void Connector::restart() {
    loop_->assertInLoopThread();
    connected_ = true;
    retryDelayMs_ = kInitRetryDelayMs;
    startInLoop();
}

}

}
