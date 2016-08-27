
#include <arale/net/EventLoop.h>
#include <arale/net/TimerID.h>
#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <unistd.h>

using namespace std;
using namespace arale;
using namespace arale::net;
//using namespace arale::base;

EventLoop *g_loop;
mutex g_mutex;

void timeout() {
    lock_guard<mutex> guard(g_mutex);
    cout<<"timeout happened"
        <<endl;
}

void cancelTimer(EventLoop *loop, TimerID timer) {
    sleep(10);
    {
        lock_guard<mutex> guard(g_mutex);
        cout<<"cancel the timer"
            <<endl;
    }
    loop->cancelTimer(timer);
}
int main() {

    EventLoop loop;
    g_loop = &loop;
    //TimerID timer = loop.runEvery(3, bind(timeout));
    TimerID timer = loop.runEvery(3, timeout);
    thread thread(bind(cancelTimer, g_loop, timer));
    thread.detach();
    loop.loop();
}
