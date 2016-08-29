
#include <arale/net/Acceptor.h>
#include <arale/net/SocketsOps.h>
#include <arale/net/InetAddress.h>

#include <functional>

namespace arale {

namespace net {

Acceptor::Acceptor(EventLoop * loop, const InetAddress& listenAddr, bool reuseport) :
    loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
    accceptChannel_(loop, acceptSocket_.getSockfd()),
    isListenning_(false)
{   
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    accceptChannel_.setReadCallback(std::bind(&Acceptor::handleNewConnection, this));
}

}

}
