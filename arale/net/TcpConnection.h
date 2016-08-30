
#ifndef ARALE_NET_TCPCONNECTION_H
#define ARALE_NET_TCPCONNECTION_H

#include <arale/net/Callbacks.h>

#include <string>
#include <memory>

namespace arale {

namespace net {

class EventLoop;
class Channel;
class Socket;

class TcpConnection {
public:
    TcpConnection(EventLoop *loop, const std::string& name, int sockfd);
    TcpConnection& TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& getName() const { return name_; }

private:
    EventLoop *loop_;
    const std::string name_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> readWriteChannel_;
};

}

}

#endif
