
#ifndef ARALE_EXAMPLE_SIMPLE_TIME_CLIENT_H
#define ARALE_EXAMPLE_SIMPLE_TIME_CLIENT_H

#include <arale/net/TcpClient.h>

class TimeClient {
public:
    TimeClient(arale::net::EventLoop *loop, const arale::net::InetAddress &addr);
    void connect();
    
private:
    void onConnection(const arale::net::TcpConnectionPtr &conn);
    void onMessage(const arale::net::TcpConnectionPtr &conn,
                arale::net::Buffer *buf, arale::Timestamp receiveTime);
    arale::net::EventLoop *loop_;
    arale::net::TcpClient client_;
};

#endif
