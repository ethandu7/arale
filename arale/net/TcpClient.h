
#ifndef ARALE_NET_TCPCLIENT_H
#define ARALE_NET_TCPCLIENT_H

#include <arale/net/Callbacks.h>

#include <memory>
#include <mutex>

namespace arale {

namespace net {

class EventLoop;
class Connector;
class InetAddress;

class TcpClient {
public:
    TcpClient(EventLoop*, const std::string&, const InetAddress&);
    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;
    ~TcpClient();

    TcpConnctionPtr getConnection() {
        std::lock_guard<std::mutex> guard(mutex_);
        return connection_;
    }

    EventLoop* getLoop() { return loop_; }
    bool retry() { return retry_; }
    void enableRetry() { retry_ = true; }

    void connect();
    // half close
    void disconnect();
    void stop();
    void newConnection(int sockfd);

    void setConnectionCallback(const ConnectionCallback &callback) {
        connectionCallback_ = callback;
    }

    void setMessageCallback(const MessageCallback &callback) {
        messageCallback_ = callback;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &callback) {
        writeCompleteCallback_ = callback;
    }
        
private:
    void removeConnection(const TcpConnctionPtr&);
    
    EventLoop* loop_;
    const std::string name_;
    int connID_;
    bool retry_;
    bool connect_;
    std::unique_ptr<Connector>  connector_;
    std::mutex mutex_;
    TcpConnctionPtr connection_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

}

}

#endif
