
#ifndef ARALE_NET_ACCEPTOR_H
#define ARALE_NET_ACCEPTER_H

#include <arale/net/Channel.h>
#include <arale/net/Socket.h>

#include <functional>


namespace arale {

namespace net {

class InetAddress;
class EventLoop;

class Acceptor {
public:
    typedef std::function<void (int sockFd, const InetAddress&)>  NewConnectionCallback;
    Acceptor(EventLoop* loop, const InetAddress&, bool);
    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &callback) {
        newConnectionCallback_ = callback;
    }

    bool isListenning() const { return isListenning_; }
    bool startListening();
    
private:
    void handleNewConnection();

    EventLoop *loop_;
    Socket acceptSocket_;
    Channel accceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool isListenning_;
};

}

}

#endif
