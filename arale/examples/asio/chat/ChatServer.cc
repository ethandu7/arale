
#include "Codec.h"

#include <arale/net/TcpServer.h>
#include <arale/net/EventLoop.h>
#include <arale/net/InetAddress.h>

#include <set>

using namespace arale;
using namespace arale::net;
using namespace arale::base;
using namespace std::placeholders;

class ChatServer {
public:
    ChatServer(EventLoop *loop, const InetAddress &addr) 
        : server_(loop, "Chat Server", addr),
          codec_(std::bind(&ChatServer::onMessage, this, _1, _2, _3)) {
        server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    }

    void start() {
        server_.start();
    }

    void onConnection(const TcpConnectionPtr &conn) {
        LOG_INFO << "Chat Server - " << conn->remoteAddr().toIpPort() << " -> "
                 << conn->localAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");
        if (conn->isConnected()) {
            // there is a bug about source insight, -:)
            connections_.insert(conn);
        } else {
            connections_.erase(conn);
        }
    }

    void onMessage(const TcpConnectionPtr &conn, const std::string &msg, Timestamp receiveTime) {
        for (auto it = connections_.begin(); it != connections_.end(); ++it) {
	        codec_.sendMessage(*it, msg);
	    }
    }
private:
    typedef std::set<TcpConnectionPtr>  ConnectionList;
    TcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionList connections_;
};

int main() {
    LOG_INFO << "pid = " << getCurrentThreadID();
    EventLoop loop;
    InetAddress addr(2016);
    ChatServer server(&loop, addr);
    server.start();
    loop.loop();
}

