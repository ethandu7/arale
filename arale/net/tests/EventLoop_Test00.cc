
#include <arale/net/EventLoop.h>
#include <thread>

using namespace arale;
using namespace arale::net;
using namespace std;

EventLoop *g_loop;

void anotherThread() {

    g_loop->loop();

}

int main() {

    EventLoop loop;
    g_loop = &loop;    
    // the grogram should crash
    thread t(anotherThread);
    // whitout this line, the grogram will crash when a thread object is destructing
    t.join();
    return 1;

}
