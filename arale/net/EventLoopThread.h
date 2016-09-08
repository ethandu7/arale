
#ifndef ARALE_NET_EVENTLOOPTHREAD_H
#define ARALE_NET_EVENTLOOPTHREAD_H

#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace arale {

namespace net {

class EventLoop;

class EventLoopThread {
public:
    typedef std::function<void (EventLoop*)> ThreadInitCallback;
    EventLoopThread(const ThreadInitCallback &callback);
    EventLoopThread(const EventLoopThread&) = delete;
    EventLoopThread& operator=(const EventLoopThread&) = delete;
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();
    EventLoop* loop_;
    bool exiting_;
    bool start_;
    std::mutex mutex_;    
    std::mutex mutex2_;
    std::condition_variable con_;
    std::condition_variable con2_;
    ThreadInitCallback initCallback_;
    std::thread thread_;
};

}

}

#endif
