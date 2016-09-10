
#include <arale/net/EventLoopThreadPool.h>
#include <arale/net/EventLoop.h>
#include <arale/net/EventLoopThread.h>
#include <stdio.h>


namespace arale {

namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop *loop, const string &name) 
    : baseLoop_(loop),
      name_(name),
      numThread_(0),
      start_(false),
      next_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {

}

void EventLoopThreadPool::start(const ThreadInitCallback &callback) {
    assert(!start_);
    baseLoop_->assertInLoopThread();
    start_ = true;
    for (int i = 0; i < numThread_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread *thread = new  EventLoopThread(buf, callback);
        threads_.push_back(thread);
        loops_.push_back(thread->startLoop());
    }

    if (numThread_ == 0 && callback) {
        callback(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;
    if (!loops_.empty()) {
        loop = loops_[next_++];
        if (implicit_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    assert(start_);
    baseLoop_->assertInLoopThread();
    if(loops_.empty()) {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    return loops_;
}

}

}
