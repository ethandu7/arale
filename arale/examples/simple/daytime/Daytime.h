
#ifndef ARALE_EXAMPLE_SIMPLE_DAYTIME_H
#define ARALE_EXAMPLE_SIMPLE_DAYTIME_H

#include <arale/net/TcpServer.h>

class DaytimeServer {
public:
    DaytimeServer(arale::net::EventLoop *loop,
                const arale::net::InetAddress &listenAddr);
    void start();
    
private:
    void onConnection(const arale::net::TcpConnectionPtr &conn);
    void onMessage(const arale::net::TcpConnectionPtr &conn,
                arale::net::Buffer *buf, arale::Timestamp time);
    arale::net::TcpServer server_;
};

#endif
