
#ifndef ARALE_NET_TCPSERVER_H
#define ARALE_NET_TCPSERVER_H

#include <arale/base/Atomic.h>
#include <arale/net/TcpConnection.h>

#include <memory>
#include <map>

namespace arale {

namespace net {

class EventLoop;
// we don't need this because in .cc we don't use this
//class InetAddress;
class Acceptor;

class TcpServer {
public:
    // for thread pool, not right now
    //typedef std::function<void (EventLoop*)> ThreadInitCallback; 

    enum Option {
        kNoReusePort,
        kReusePort
    };
    
    TcpServer(EventLoop *loop, 
            const std::string &name, 
            const InetAddress &serverAddr,
            Option option = kNoReusePort);
    TcpServer(const TcpServer&) = delete;
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
    void newConncetion(int newconnfd, const InetAddress& peeraddr); 
    // key is the name of TcpConnection
    typedef std::map<std::string, TcpConnctionPtr> ConnectionMap;
    EventLoop *loop_;
    const std::string name_;
    const std::string ipPort_;
    int nextConnId_;
    std::unique_ptr<Acceptor> accptor_;
    ConnectionMap connections_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};


}

}

#endif
