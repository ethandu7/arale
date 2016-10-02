
#ifndef ARALE_EXAMPLE_MAXCONNECTION_ECHO_H
#define ARALE_EXAMPLE_MAXCONNECTION_ECHO_H

#include <arale/net/TcpServer.h>

class EchoServer {
public:
    EchoServer(arale::net::EventLoop *, const arale::net::InetAddress &, int);
    void  start();
    
private:
    void onMessage(const arale::net::TcpConnectionPtr &,
                    arale::net::Buffer *,
                    arale::Timestamp);
    void onConnection(const arale::net::TcpConnectionPtr &);
    
    arale::net::TcpServer server_;
    int numConnection_;
    const int maxConnections_;
};

#endif
