
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

    const std::string& getIpPort() { return ipPort_; }

    const std::string& getName() { return name_; }

    EventLoop* getLoop() { return loop_; }

    void start();

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
    void removeConnection(const TcpConnctionPtr&);
    void removeConnectionInLoop(const TcpConnctionPtr&);
    // key is the name of TcpConnection
    typedef std::map<std::string, TcpConnctionPtr> ConnectionMap;
    EventLoop *loop_;
    const std::string name_;
    const std::string ipPort_;
    int nextConnId_;
    std::unique_ptr<Acceptor> accptor_;
    ConnectionMap connections_;
    AtomicInt32 started_;
    // three and a half events need to be concerned in networking programming(according to Chen Shou)
    // 1.  new connection accepted
    // 2.  existing connection close(passive or initiative)
    // 3.  get data from peer
    // 3.5 finish writing data to peer
    // here we give the library user a chance to customize the handler for event 1, 3 and 3.5
    // but we will fully control what would happen when event 2 happens
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};


}

}

#endif
