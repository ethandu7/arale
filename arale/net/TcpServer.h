
#ifndef ARALE_NET_TCPSERVER_H
#define ARALE_NET_TCPSERVER_H

#include <arale/base/Atomic.h>
#include <arale/net/TcpConnection.h>

#include <memory>
#include <map>

namespace arale {

namespace net {

class EventLoop;
class InetAddress;
class Acceptor;

class TcpServer {
public:
    typedef std::function<void ()> 
    
    TcpServer(EventLoop *loop, const InetAddress &serverAddr);
    TcpServer& TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;
    ~TcpServer();

    void setConnectionCallback(const ConnectionCallback& callback) {
        connectionCallback_ = callback;
    }

    void setMessageCallback(const MessageCallback& callback) {
        messageCallback_ = callback;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& callback) {
        writeCompleteCallback_ = callback;
    }
    
private:
    // key is the name of TcpConnection
    typedef std::map<std::string, TcpConnctionPtr> ConnectionMap;
    EventLoop *loop_;
    std::unique_ptr<Acceptor> accptor_;
    ConnectionMap connections_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};


}

}

#endif
