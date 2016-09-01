
#include <arale/net/Acceptor.h>
#include <arale/net/SocketsOps.h>
#include <arale/net/InetAddress.h>
#include <arale/net/EventLoop.h>

#include <functional>

#include <errno.h>
#include <fcntl.h>

namespace arale {

namespace net {

Acceptor::Acceptor(EventLoop * loop, const InetAddress &listenAddr, bool reuseport) :
    loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
    accceptChannel_(loop, acceptSocket_.getSockfd()),
    isListening_(false)
{   
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    accceptChannel_.setReadCallback(std::bind(&Acceptor::handleNewConnection, this));
    backupFd_ = open("/dev/null", O_RDONLY | O_CLOEXEC);
}

Acceptor::~Acceptor() {
    if (isListening_) {
        accceptChannel_.disableAll();
        accceptChannel_.remove();
    }
    close(backupFd_);
}

void Acceptor::startListening() {
    loop_->assertInLoopThread();
    acceptSocket_.listen();
    accceptChannel_.enableRead();
    isListening_ = true;
}

void Acceptor::handleNewConnection() {
    loop_->assertInLoopThread();
    InetAddress peeraddr;
    int newconnfd = acceptSocket_.accept(&peeraddr);
    if (newconnfd > 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(newconnfd, peeraddr);
        } else {
            sockets::close(newconnfd);
        }
    } else {
        LOG_SYSERR << "[Acceptor::handleNewConnection]: cann't get new connection";
        if (errno == EMFILE) {
            close(backupFd_);
            backupFd_ = acceptSocket_.accept(&peeraddr);
            // let client know we want to close this connection
            close(backupFd_);
            // get the backup back
            backupFd_ = open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

}

}
