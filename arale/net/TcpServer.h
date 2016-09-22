
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
class EventLoopThreadPool;

class TcpServer {
public:
    typedef std::function<void (EventLoop*)> ThreadInitCallback; 

    enum Option {
        kNoReusePort,
        kReusePort
    };

    // a user must construct a EventLoop object first
    // then he can construct a TcpServer object
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
    void setThreadNum(int num);

    void setConnectionCallback(const ConnectionCallback& callback) {
        connectionCallback_ = callback;
    }

    void setMessageCallback(const MessageCallback& callback) {
        messageCallback_ = callback;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& callback) {
        writeCompleteCallback_ = callback;
    }

    void setThreadInitCallback(const ThreadInitCallback &callback) {
        threadInitCallback_ = callback;
    }
     
private:
    void newConncetion(int newconnfd, const InetAddress& peeraddr);
    void removeConnection(const TcpConnectionPtr&);
    // if a class define any inloop version function, this funnction 
    // can be call from a thread is different from io thread
    //
    // only if you want to add or remove data from loop object you need 
    // define a inloop version for your function
    void removeConnectionInLoop(const TcpConnectionPtr&);
    // key is the name of TcpConnection
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
    EventLoop *loop_;
    const std::string name_;
    const std::string ipPort_;
    int nextConnId_;
    std::unique_ptr<Acceptor> accptor_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    ConnectionMap connections_;
    AtomicInt32 started_;
    // three and a half events need to be concerned in networking programming(according to Chen Shou)
    // 1.  new connection accepted
    // 2.  existing connection close(passive or initiative)
    // 3.  get data from peer
    // 3.5 finish writing data to peer
    // here we give the library user a chance to customize the handler for event 3 and 3.5
    // but we will fully control what would happen when event 1 and 2 happens
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
};


}

}

#endif
