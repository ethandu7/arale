
#include <arale/base/Logging.h>
#include <arale/net/TcpConnection.h>
#include <arale/net/Socket.h>
#include <arale/net/Channel.h>

namespace arale {

namespace net {

void defaultConnectionCallback(const TcpConnctionPtr &conn) {
    LOG_TRACE << "defaultConnectionCallback";
}

void defaultMessageCallback(const TcpConnctionPtr & conn, Buffer *buf, Timestamp receiveTime) {
    
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

}

}

}
