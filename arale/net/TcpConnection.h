
#ifndef ARALE_NET_TCPCONNECTION_H
#define ARALE_NET_TCPCONNECTION_H

#include <arale/net/Callbacks.h>
#include <arale/net/InetAddress.h>
#include <arale/net/Buffer.h>
#include <arale/net/Socket.h>

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

    enum State { kConnecting, kConnected, kDisconnecting, kDisconnected };

    void setState(State state) { state_ = state; }
    EventLoop* getLoop() const { return loop_; }
    const std::string& getName() const { return name_; }

    const InetAddress& localAddr() { return localAddr_; }
    const InetAddress& remoteAddr() { return remoteAddr_; }

    bool isConnected() { return state_ == kConnected; }
    bool isReading() { return reading_; }

    // those setters are called only by TcpServer and TcpClient
    // that means users should provide callbacks before get or send data
    // do NOT set those callbacks by using TcpConnction object directly
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

    bool getTcpInfo(struct tcp_info *tcpi) const ;
    std::string getTcpInfoString() const ;
    
    void connectionEstablished();
    void connectionDestroyed();

    void send(const void* data, size_t len);
    void shutdown();
    void startRead();
    void stopRead();
    void forceClose();
    void forceCloseWithDelay(double seconds);

    const char* stateToString() const;
    void setTcpNoDelay(bool on) { socket_->setTcpNoDelay(on); }
    
private:
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void shutdownInLoop();
    void sendInLoop(const void *data, size_t len);
    void startReadInLoop();
    void stopReadInLoop();
    void forceCloseInLoop();
    
    EventLoop *loop_;
    const std::string name_;
    State state_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> readWriteChannel_;
    InetAddress localAddr_;
    InetAddress remoteAddr_;
    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    bool reading_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
};

}

}

#endif
