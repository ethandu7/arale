
#include <arale/net/EventLoop.h>
#include <arale/net/TcpServer.h>
#include <arale/net/InetAddress.h>

#include <thread>

using namespace arale;
using namespace net;
using namespace std;

EventLoop *g_loop;

void threadFunc() {
    InetAddress addr(8080);
    TcpServer server(g_loop, "no-crash-server", addr);
    server.start();
    while (true)
        ;
}

int main () {
    EventLoop loop;
    g_loop = &loop;
    thread t(threadFunc);
    loop.loop();
    //t.join();
    return 0;
}
