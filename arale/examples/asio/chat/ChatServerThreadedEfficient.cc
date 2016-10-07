
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
              codec_(std::bind(&ChatServer::onMessage, this, _1, _2, _3)),
              connections_(new ConnectionList) {
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
    // we don't have to put all typedef at the bgeinning any more
    typedef std::set<TcpConnectionPtr>  ConnectionList;
    typedef std::shared_ptr<ConnectionList> ConnectionListPtr;
    
    void onConnection(const TcpConnectionPtr &conn) {
        LOG_INFO << "Chat Server - " << conn->remoteAddr().toIpPort() << " -> "
                 << conn->localAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");

        // notice: the lock should guard the whole function
        // since there is chance when we write the connection list while
        // some other threads want to read this list
        
        // if other thread is reading the conection list
        // we copy the list, then do insertion or erasure on the new copy
        std::lock_guard<std::mutex> guard(mutex_);
        if (!connections_.unique()) {
            connections_.reset(new ConnectionList(*connections_));
        }
        assert(connections_.unique());

        if (conn->isConnected()) {
            // there is a bug about source insight, -:)
            connections_->insert(conn);
        } else {
            connections_->erase(conn);
        }
    }

    void onMessage(const TcpConnectionPtr &conn, const std::string &msg, Timestamp receiveTime) {
        // this will increase the reference count, which tells the writer we are reading

        ConnectionListPtr connections;
        {
            std::lock_guard<std::mutex> guard(mutex_);
            connections = connections_;
        }

        for (auto it = connections->begin(); it != connections->end(); ++it) {
	        codec_.sendMessage(*it, msg);
	    }
    }
    
    TcpServer server_;
    LengthHeaderCodec codec_;
    std::mutex mutex_;
    ConnectionListPtr connections_;
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

