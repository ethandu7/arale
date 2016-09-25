
#ifndef ARALE_EXAMPLE_SIMPLE_CHARGEN_H
#define ARALE_EXAMPLE_SIMPLE_CHARGEN_H

#include <arale/net/TcpServer.h>
#include <string>

// RFC 864
class ChargenServer {
public:
    ChargenServer(arale::net::EventLoop *loop, 
                const arale::net::InetAddress &addr, bool print);
    void start();
    
private:
    void onConnection(const arale::net::TcpConnectionPtr &conn);
    void onMessage(const arale::net::TcpConnectionPtr &conn, 
                arale::net::Buffer *buf, arale::Timestamp time);
    void onWriteComplete(const arale::net::TcpConnectionPtr &conn);
    void printThroughput();
    
    std::string message_;
    arale::net::TcpServer server_;
    int64_t transferred_;
    arale::Timestamp startTime_;
};

#endif 
