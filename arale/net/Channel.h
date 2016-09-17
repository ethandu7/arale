
#ifndef ARALE_NET_CHANNEL_H
#define ARALE_NET_CHANNEL_H

#include <arale/base/TimeStamp.h>
#include <functional>
#include <memory>

namespace arale {

namespace net {

class EventLoop;

class Channel {
public:
    // not support by g++ 4.4.6
    //using EventCallback = std::function<void ()>;
    //using ReadEventCallback = std::function<void (Timestamp)>;
    typedef std::function<void ()>  EventCallback;
    typedef std::function<void (Timestamp)> ReadEventCallback;
    
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
    void setCloseCallback(const EventCallback &closeEventCallback) 
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
    // for poller
    void setIndex(int index) { index_ = index; }
    int getIndex() { return index_; }
    int getEvents() { return events_; }
    bool isReading() { return events_ & kReadEvent; }
    bool isWriting() { return events_ & kWriteEvent; }
    bool isNoneEvent() { return events_ == kNoneEvent; }
    EventLoop* getEventLoop() { return loop_; }

    void handleEvent(Timestamp receiveTime);

    void remove();

    // why can't the type of parameter be weak_ptr???
    void tieTo(const std::shared_ptr<void> &obj);

    std::string eventsToString();
    std::string eventsToString(int fd, int event);

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);
    
    EventLoop *loop_;
    const int fd_;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;

    // used by poller
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    int events_;
    int revents_;
    bool isInLoop_;
    // the index in the poller's fd set
    int index_;
    // make sure the object who is having us hasn't been destoryed
    // before we are trying to handle event
    std::weak_ptr<void> tie_;
    bool tied_;
    // make sure we will not be destoryed during handle event
    bool isHandlingEvent_;
};

}

}

#endif
