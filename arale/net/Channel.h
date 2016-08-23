
#ifndef ARALE_NET_CHANNEL_H
#define ARALE_NET_CHANNEL_H

#include <arale/base/TimeStamp.h>
#include <functional>

namespace arale {

namespace net {

class EventLoop;

class Channel {
public:
    using EventCallback = std::function<void ()>;
    using ReadEventCallback = std::function<void (Timestamp)>;
    
    Channel(EventLoop* loop, int socketFD);
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    ~Channel();
 
    void setReadCallback(const ReadEventCallback &readEventCallback) 
    { readCallback_ = readEventCallback; }
    void setWriteCallback(const EventCallback &writeEventCallback) 
    { writeCallback_ = writeEventCallback; }
    void setErrorCallback(const EventCallback &errorEventCallback) 
    { errorCallback_ = errorEventCallback; }
    void setColseCallback(const EventCallback &closeEventCallback) 
    { closeCallback_ = closeEventCallback; }

    void enableRead()
    { events_ |= kReadEvent; update(); }
    void disableRead()
    { events_ &= ~kReadEvent; update(); }
    void enableWrite()
    { events_ |= kWriteEvent; update(); }
    void disableWrite()
    { events_ &= kWriteEvent; update(); }
    void disableAll()
    { events_ = kNoneEvent; update(); }

    void set_revents(int revents) { revents_ = revents; }

    int getfd() { return fd_; }

    void HandleEvent(Timestamp receiveTime);

    void remove();

private:
    void update();
    
    EventLoop *loop_;
    const int fd_;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
    
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    int events_;
    int revents_;
    bool isInLoop;
};

}

}

#endif
