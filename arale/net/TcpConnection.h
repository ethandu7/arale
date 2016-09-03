
#ifndef ARALE_NET_TCPCONNECTION_H
#define ARALE_NET_TCPCONNECTION_H

#include <arale/net/Callbacks.h>
#include <arale/net/InetAddress.h>

#include <string>
#include <memory>

namespace arale {

namespace net {

class EventLoop;
class Channel;
class Socket;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop, const std::string& name, int sockfd,
                    const InetAddress &localaddr, const InetAddress &remoteaddr);
    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;
    ~TcpConnection();

    enum State { kConnecting, kConnected };

    void setState(State state) { state_ = state; }
    EventLoop* getLoop() const { return loop_; }
    const std::string& getName() const { return name_; }

    const InetAddress& localAddr() { return localAddr_; }
    const InetAddress& remoteAddr() { return remoteAddr_; }

    bool isConnected() { return state_ == kConnected; }

    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback &cb) {
        messageCallback_ = cb;
    }
    
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
        writeCompleteCallback_ = cb;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb) {
        highWaterMarkCallback_ = cb;
    }

    void setCloseCallback(const CloseCallback &cb) {
        closeCallback_ = cb;
    }

    void connectionEstablished();
    void connectionDestroyed();
private:
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    
    EventLoop *loop_;
    const std::string name_;
    State state_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> readWriteChannel_;
    InetAddress localAddr_;
    InetAddress remoteAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
};

}

}

#endif