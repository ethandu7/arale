
#include "Codec.h"

#include <arale/net/TcpClient.h>
#include <arale/net/EventLoopThread.h>
// god damn it, this must need for arale::base namespace
#include <arale/net/EventLoop.h>

#include <stdio.h>
#include <iostream>
#include <unistd.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;
using namespace std::placeholders;

class ChatClient {
public:
    ChatClient(EventLoop *loop, const InetAddress &addr) 
        : client_(loop, "Chat Client", addr),
          codec_(std::bind(&ChatClient::onStringMessage, this, _1, _2, _3)) {
        client_.setConnectionCallback(std::bind(&ChatClient::onConnection, this, _1));
        client_.setMessageCallback(std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
        client_.enableRetry();
    }

    void connect() {
        client_.connect();
    }

    void disconnect() {
        client_.disconnect();
    }

    void sendMessage(const std::string &msg) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (connection_) {
            codec_.sendMessage(connection_, msg);
        }
    }
    
private:   
    void onConnection(const TcpConnectionPtr & conn) {
        LOG_INFO << "TimeClient - " << conn->localAddr().toIpPort() << " -> "
                 << conn->remoteAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");
        std::lock_guard<std::mutex> gurad(mutex_);
        if (conn->isConnected()) {
            connection_ = conn;
        } else {
            connection_.reset();
        }
    }
    
    void onStringMessage(const TcpConnectionPtr &conn, const std::string &msg, Timestamp receiveTime) {
        // can not use std::cout, it's not thread-safe
        printf("<<<%s\n", msg.c_str());
    }
    
    TcpClient client_;
    LengthHeaderCodec codec_;
    TcpConnectionPtr connection_;
    std::mutex mutex_;
};

int main(int argc, char *argv[]) {
    LOG_INFO << "pid = " << getCurrentThreadID();
    if (argc > 1) {
        EventLoopThread loopthread;
        InetAddress serverAddr(argv[1], 2016);
        ChatClient client(loopthread.startLoop(), serverAddr);
        client.connect();

        std::string line;
        while (std::getline(std::cin, line)) {
            client.sendMessage(line);
        }

        client.disconnect();
        // should use std::chrono
        sleep(1);
    } else {
        printf("Usage: %s host_ip\n", argv[0]);
    }
}
