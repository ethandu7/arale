
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
    void startListening();
    
private:
    void handleNewConnection();

    EventLoop *loop_;
    Socket acceptSocket_;
    Channel accceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool isListenning_;
    // it will be created at the beginning
    // when the FD runs out(accept cann't get a FD) 
    // we can close this idle FD and make accept get a FD
    // then we close the new created FD, so client can get something at least
    // the last step is to creat a backup FD again, just like beginning
    int backupFd_;
};

}

}

#endif
