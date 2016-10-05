
#include "Codec.h"

#include <arale/net/EventLoop.h>
#include <arale/net/TcpServer.h>
#include <arale/net/InetAddress.h>

#include <set>
#include <mutex>
#include <stdio.h>

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

        void setThreadNum(int threadNum) {
            server_.setThreadNum(threadNum);
        }

private:
    typedef std::set<TcpConnectionPtr>  ConnectionList;

    void onConnection(const TcpConnectionPtr &conn) {
        LOG_INFO << "Chat Server - " << conn->remoteAddr().toIpPort() << " -> "
                 << conn->localAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");

        // the execution will hang up here when another thread is sending out message
        // at this moment, the connection is establised, we are waiting for the first data from client
        std::lock_guard<std::mutex> guard(mutex_);
        if (conn->isConnected()) {
            // there is a bug about source insight, -:)
            connections_.insert(conn);
        } else {
            connections_.erase(conn);
        }
    }

    void onMessage(const TcpConnectionPtr &conn, const std::string &msg, Timestamp receiveTime) {
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto it = connections_.begin(); it != connections_.end(); ++it) {
	        codec_.sendMessage(*it, msg);
	    }
    }
    
    TcpServer server_;
    LengthHeaderCodec codec_;
    std::mutex mutex_;
    ConnectionList connections_;
};

int main(int argc, char *argv[]) {
    LOG_INFO << "pid = " << getCurrentThreadID();
    if (argc > 1) {
        EventLoop loop;
        InetAddress addr(2016);
        ChatServer server(&loop, addr);
        int num = atoi(argv[1]);
        server.setThreadNum(num);
        server.start();
        loop.loop();

    } else {
        printf("Usage: %s thread_num\n", argv[0]);
    }
}

