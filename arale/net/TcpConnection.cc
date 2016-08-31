
#include <arale/net/TcpConnection.h>
#include <arale/net/Socket.h>
#include <arale/net/Channel.h>

namespace arale {

namespace net {

TcpConnection::TcpConnection(EventLoop *loop, const string &name, int sockfd) 
    : loop_(loop),
      name_(name),
      socket_(new Socket(sockfd)),
      readWriteChannel_(new Channel(loop_, sockfd)) {

}

}

}
