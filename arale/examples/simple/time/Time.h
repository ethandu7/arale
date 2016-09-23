
#ifndef ARALE_EXAMPLE_SIMPLE_TIME_H
#define ARALE_EXAMPLE_SIMPLE_TIME_H

#include <arale/net/TcpServer.h>

class TimeServer {
public:
    TimeServer(arale::net::EventLoop *loop, const arale::net::InetAddress &addr);
    void start();
    
private:
    void onConnection(const arale::net::TcpConnectionPtr &conn);
    void onMessage(const arale::net::TcpConnectionPtr &conn, 
                arale::net::Buffer *buf, arale::Timestamp time);
    arale::net::TcpServer server_;
};

#endif
