
#include <arale/base/Logging.h>
#include <arale/net/EventLoop.h>
#include <arale/net/TcpClient.h>
#include <arale/net/InetAddress.h>

#include <functional>
#include <stdio.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

class ChargenClient {
public:
    ChargenClient(EventLoop *loop, const InetAddress &addr)
        : loop_(loop),
          client_(loop, "Chargen Client", addr) {
          using namespace std::placeholders;
        client_.setConnectionCallback(std::bind(&ChargenClient::onConnection, this, _1));
        client_.setMessageCallback(std::bind(&ChargenClient::onMessage, this, _1, _2, _3));
    }
    void connect() {
        client_.connect();
    }
private:
    void onConnection(const TcpConnectionPtr &conn) {
        LOG_INFO << "TimeClient - " << conn->localAddr().toIpPort() << " -> "
                 << conn->remoteAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");
        if (!conn->isConnected()) {
            loop_->quitLoop();
        }
    }
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
        buf->retrieveAll();
    }
    
    EventLoop *loop_;
    TcpClient client_;
};


int main(int argc, char *argv[]) {
    LOG_INFO << "pid = " << getCurrentThreadID();
    if (argc > 1)
    {
        EventLoop loop;
        InetAddress serverAddr(argv[1], 2016);
        ChargenClient chargenClient(&loop, serverAddr);
        chargenClient.connect();
        loop.loop();
    }
    else
    {
        printf("Usage: %s host_ip\n", argv[0]);
    }
}

