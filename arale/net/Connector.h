
#ifndef ARALE_NET_CONNECTOR_H
#define ARALE_NET_CONNECTOR_H

#include <arale/net/InetAddress.h>
#include <memory>

namespace arale {

namespace net {

class EventLoop;
class Channel;

class Connector : public std::enable_shared_from_this<Connector> {
public:
    typedef std::function<void (int)> NewConnectionCallback;

    enum State { kDisconnected, kConnecting, kConnected };
    
    Connector(EventLoop *loop, const InetAddress &serverAddr);
    Connector(const Connector&) = delete;
    Connector& operator=(const Connector&) = delete;
    ~Connector();

    void start();
    void restart();
    void stop();
    void setNewConnectionCallback(const NewConnectionCallback &callback) {
        connectionCallback_ = callback;
    }

    void setState(State s) { state_ = s; }

private:
    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;
    void retry(int sockfd);
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    void handleWrite();
    void handleError();
    
    EventLoop *loop_;
    InetAddress serverAddr_;
    std::unique_ptr<Channel> connectChannel_;
    // will be called after connection is establised, before get any data from peer
    NewConnectionCallback connectionCallback_;
    State state_;
    bool connected_;
    int retryDelayMs_;
};

}

}

#endif
