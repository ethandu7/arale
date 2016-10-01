
#ifndef ARALE_NET_EVENTLOOPTHREAD_H
#define ARALE_NET_EVENTLOOPTHREAD_H

#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>

namespace arale {

namespace net {

class EventLoop;

class EventLoopThread {
public:
    typedef std::function<void (EventLoop*)> ThreadInitCallback;
    EventLoopThread(const std::string &name = std::string(), const ThreadInitCallback &callback = ThreadInitCallback());
    EventLoopThread(const EventLoopThread&) = delete;
    EventLoopThread& operator=(const EventLoopThread&) = delete;
    ~EventLoopThread();
    EventLoop* startLoop();

    const std::string getName() { return name_; }

private:
    void threadFunc();
    EventLoop* loop_;
    std::string name_;
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
