
#include "Codec.h"

#include <arale/base/ThreadLocalSingleton.h>
#include <arale/net/TcpServer.h>
#include <arale/net/EventLoop.h>

#include <set>
#include <mutex>

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
        server_.setThreadInitCallback(std::bind(&ChatServer::threadInit, this, _1));
        server_.setThreadNum(threadNum);
    }

    void threadInit(EventLoop *loop) {
        assert(LocalConnections::pointer() == NULL);
        LocalConnections::instance();
        assert(LocalConnections::pointer() != NULL);
        std::lock_guard<std::mutex> guard(mutex_);
        loops_.insert(loop);
    }

private:
    typedef std::set<TcpConnectionPtr>  ConnecitonList;
    typedef ThreadLocalSingleton<ConnecitonList> LocalConnections;
    
    void onConnection(const TcpConnectionPtr &conn) {
        LOG_INFO << "Chat Server - " << conn->remoteAddr().toIpPort() << " -> "
                 << conn->localAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");

        if (conn->isConnected()) {
            LocalConnections::instance().insert(conn);
        } else {
            LocalConnections::instance().erase(conn);
        }
    }

    void distributeMessage(const std::string &msg) {
        LOG_INFO << "begin send messages";
        for (auto it = LocalConnections::instance().begin(); it != LocalConnections::instance().end(); ++it) {
            codec_.sendMessage(*it, msg);
        }
        LOG_INFO << "end";
    }

    void onMessage(const TcpConnectionPtr &conn, const  string &msg, Timestamp receiveTime) {
        LOG_INFO << "post send action to every thread";
        EventLoop::InLoopFunctor func = std::bind(&ChatServer::distributeMessage, this, msg);
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto it = loops_.begin(); it != loops_.end(); ++it) {
            (*it)->postFuntor(func);
        }
    }
    
    TcpServer server_;
    LengthHeaderCodec codec_;
    // this is used to protect loops_
    std::mutex mutex_;
    std::set<EventLoop *> loops_;
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

