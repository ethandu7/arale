
#include <arale/net/EventLoopThread.h>
#include <arale/net/EventLoop.h>

namespace arale {

namespace net {

EventLoopThread::EventLoopThread(const ThreadInitCallback &callback)
    : loop_(NULL),
      exiting_(false),
      start_(false),
      initCallback_(callback),
      thread_(std::bind(&EventLoopThread::threadFunc, this)) {
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != NULL) {
        loop_->quitLoop();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    assert(loop_ == NULL);

    // since the std::thread will start to run the execution immediately after 
    // the thread object is created, we need make it wait on a condition variable,
    // and at here we do the notification to let the execution keep going
    {
        std::lock_guard<std::mutex> guard(mutex2_);
        start_ = true;
        // the notified thread will get the notification immediately 
        // but notify doesn't unlock the mutex
        // so the notified thread will still be blocked until we go out of this scope
        con2_.notify_one();
    }
    
    {
        // Note: don't ues lock_guard here
        // the condition variable need to unlock the mutex in wait
        // but lock_guard dosen't have this unlock interface
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == NULL) {
            con_.wait(lock);
        }
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    
    {
        std::unique_lock<std::mutex> lock(mutex2_);
        while (start_ == false) {
            con2_.wait(lock);
        }
    }
    assert(start_ == true);
    EventLoop loop;
    if (initCallback_) {
        initCallback_(&loop);
    }
    
    {
        std::lock_guard<std::mutex> guard(mutex_);
        loop_ = &loop;
        con_.notify_one();
    }

    loop.loop();
    loop_ = NULL;
}

}

}
