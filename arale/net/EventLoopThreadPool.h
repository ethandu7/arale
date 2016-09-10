
#ifndef ARALE_NET_EVENTLOOPTHREADPOOL_H
#define ARALE_NET_EVENTLOOPTHREADPOOL_H

#include <arale/base/Types.h>

#include <functional>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

namespace arale {

namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
    typedef std::function<void (EventLoop*)> ThreadInitCallback;
    EventLoopThreadPool(EventLoop *loop, const std::string& name);
    EventLoopThreadPool(const EventLoopThreadPool&) = delete;
    EventLoopThreadPool operator=(const EventLoopThreadPool&) = delete;
    ~EventLoopThreadPool();

    void setThreadNum(int numThread) { numThread_ = numThread; };
    const std::string& getName() { return name_; }
    bool isStarted() { return start_; }
    void start(const ThreadInitCallback &callback);
    std::vector<EventLoop *> getAllLoops();

    EventLoop* getNextLoop();

private:
    EventLoop *baseLoop_;
    std::string name_;
    int numThread_;
    bool start_;
    boost::ptr_vector<EventLoopThread> threads_;
    int next_;
    std::vector<EventLoop*> loops_;
};

}


}


#endif