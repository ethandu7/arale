

#ifndef ARALE_EXAMPLES_SIMPLE_DISCARD_H
#define ARALE_EXAMPLES_SIMPLE_DISCARD_H

#include <arale/net/TcpServer.h>

class DiscardServer {
public:
    DiscardServer(arale::net::EventLoop* loop,
                  const arale::net::InetAddress &serverAddr);
    void start();

private:
    void onMessage(const arale::net::TcpConnectionPtr &conn,
                    arale::net::Buffer* buf,
                    arale::Timestamp time);
    void onConnection(const arale::net::TcpConnectionPtr &conn);
    arale::net::TcpServer server_;
};


#endif

