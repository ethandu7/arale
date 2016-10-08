
#include <arale/net/Buffer.h>
#include <arale/net/TcpServer.h>
#include <arale/net/TcpClient.h>
#include <arale/net/EventLoop.h>

#include <map>
#include <queue>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

static const int kMaxConns = 10;
static const size_t kMaxPacketLen = 253;
static const size_t kHeaderLen = 3;

static const uint16_t defaultListenPort = 3333;
static const uint16_t defaultBackendPort = 9999;
static const char* defaultBackedIP = "127.0.0.1";

class MultiplexServer {
public:
    MultiplexServer(EventLoop *loop, const InetAddress &listenAddr, const InetAddress &backendAddr, int numThread) 
        : server_(loop, "Multiplex Server", listenAddr),
          backend_(loop, "Multiplex Backend", backendAddr),
          numThreads_(numThread) {        
        using namespace std::placeholders;
        server_.setConnectionCallback(std::bind(&MultiplexServer::onClientConnection, this, _1));
        server_.setMessageCallback(std::bind(&MultiplexServer::onClientMessage, this, _1, _2, _3));
        backend_.setConnectionCallback(std::bind(&MultiplexServer::onBackendConnection, this, _1));
        backend_.setMessageCallback(std::bind(&MultiplexServer::onBackendMessage, this, _1, _2, _3));
        backend_.enableRetry();
    }

    void start() {
        LOG_INFO << "starting " << numThreads_ << " threads.";
        backend_.connect();
        server_.setThreadNum(numThreads_);
        server_.start();
    }
    
private:
    void sendBackendPacket(int id, Buffer* buf) {
        size_t len = buf->readableBytes();
        LOG_DEBUG << "sendBackendPacket " << len;
        uint8_t header[kHeaderLen] = {
            static_cast<uint8_t>(len),
            static_cast<uint8_t>(id & 0xFF),
            static_cast<uint8_t>((id & 0xFF00) >> 8),
        };
        buf->prepend(header, kHeaderLen);

        // the copy can make the critical region small by moving the call of send out
        TcpConnectionPtr backendConn;
        {
            std::lock_guard<std::mutex> guard(mutex_);
            backendConn = backendConn_;
        }
        // the backend can be disconnected any time
        if (backendConn) {
            backendConn->send(buf);
        }
    }

    // no lock is on when this is called, cause we don't involve shared data here
    void sendBackendString(int id, const std::string &msg) {
        assert(msg.size() < kMaxPacketLen);
        Buffer buf;
        buf.append(msg.c_str(), msg.size());
        sendBackendPacket(id, &buf);
    }

    // no lock is on when this is called
    void sendBackendBuffer(int id, Buffer *buf) {
        // max length of packet is 256
        while (buf->readableBytes() > kMaxPacketLen) {
            Buffer buf2;
            buf2.append(buf->peek(), kMaxPacketLen);
            buf->retrieve(kMaxPacketLen);
            sendBackendPacket(id, &buf2);
        }
        if (buf->readableBytes() > 0) {
            sendBackendPacket(id, buf);
        }
    }
    
    void onClientConnection(const TcpConnectionPtr &conn) {
        LOG_INFO << "Multiplex Server Client - " << conn->remoteAddr().toIpPort() << " -> "
                 << conn->localAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");

        if (conn->isConnected()) {
            int id = -1;
            {
                std::lock_guard<std::mutex> guard(mutex_);
                if (!availIds_.empty()) {
                    id = availIds_.front();
                    availIds_.pop();
                    clientConns_[id] = conn;
                }
            }

            if (id < 0) {
                conn->shutdown();
            } else {
                conn->setContext(id);
                char buf[256];
                snprintf(buf, sizeof(buf), "CONN %d FROM %s IS UP\r\n", id, conn->remoteAddr().toIpPort().c_str());
                sendBackendString(0, buf);
            }
        } else {
            if (!conn->getContext().empty()) {
                int id = boost::any_cast<int>(conn->getContext());
                assert(id > 0 && id < kMaxConns);
                char buf[256];
                snprintf(buf, sizeof(buf), "CONN %d FROM %s IS DOWN\r\n", id, conn->remoteAddr().toIpPort().c_str());
                sendBackendString(0, buf);
                std::lock_guard<std::mutex> guard(mutex_);
                if (backendConn_) {
                    availIds_.push(id);
                    clientConns_.erase(id);
                } else {
                    assert(availIds_.empty());
                    assert(clientConns_.empty());
                }
            }
        }
    }

    void onClientMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
        if (!conn->getContext().empty()) {
            assert(!conn->getContext().empty());
            int id = boost::any_cast<int>(conn->getContext());
            sendBackendBuffer(id, buf);
        } else {
            buf->retrieveAll();
        }
    }

    void onBackendConnection(const TcpConnectionPtr &conn) {
        LOG_INFO << "Multiplex Server Backend - " << conn->localAddr().toIpPort() << " -> "
                 << conn->remoteAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");

        if (conn->isConnected()) {
            std::lock_guard<std::mutex> guard(mutex_);
            assert(availIds_.empty());
            backendConn_ = conn;
            for (int i = 1; i <= kMaxConns; ++i) {
                availIds_.push(i);
            }
        } else {
            std::lock_guard<std::mutex> guard(mutex_);
            backendConn_.reset();
            for (auto it  = clientConns_.begin(); it != clientConns_.end(); ++it) {
                it->second->shutdown();
            }
            clientConns_.clear();
            // why std::queue doesn't have clear operation
            while (!availIds_.empty()) {
                availIds_.pop();
            }
        }
    }

    void onBackendMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
        while (buf->readableBytes() > kHeaderLen) {
            size_t len = static_cast<size_t>(*buf->peek());
            if (buf->readableBytes() < len + kHeaderLen) {
                break;
            } else {
                int id = static_cast<int>(buf->peek()[1]);
                id |= (static_cast<int>(buf->peek()[2]) << 8);
                if (id != 0) {
                    TcpConnectionPtr clientConn;
                    {
                        std::lock_guard<std::mutex> guard(mutex_);
                        auto it = clientConns_.find(id);
                        if (it != clientConns_.end()) {
                            clientConn = it->second;
                        }
                    }
                    if (clientConn) {
                        clientConn->send(buf->peek() + kHeaderLen, len);
                    }
                } else {
                    // the content of buf may not have '\0', so we need provide len
                    std::string cmd(buf->peek() + kHeaderLen, len);
                    LOG_INFO << "Backend cmd " << cmd;
                    doCommand(cmd);
                }
                buf->retrieve(len + kHeaderLen);
            }
        }
    }

    void doCommand(const std::string &cmd) {
        static const std::string kDisconnectCmd = "DISCONNECT ";

        if (cmd.size() > kDisconnectCmd.size()
            && std::equal(kDisconnectCmd.begin(), kDisconnectCmd.end(), cmd.begin()))
        {
            int connId = atoi(&cmd[kDisconnectCmd.size()]);
            TcpConnectionPtr clientConn;
            {
                std::lock_guard<std::mutex> guard(mutex_);
                std::map<int, TcpConnectionPtr>::iterator it = clientConns_.find(connId);
                if (it != clientConns_.end())
                {
                    clientConn = it->second;
                }
            }
            if (clientConn) {
                clientConn->shutdown();
            }
        }
    }
    
    TcpServer server_;
    TcpClient backend_;
    int numThreads_;
    std::mutex mutex_;
    TcpConnectionPtr backendConn_;
    std::map<int, TcpConnectionPtr> clientConns_;
    std::queue<int> availIds_;
};

int main(int argc, char *argv[]) {
    LOG_INFO << "pid = " << getCurrentThreadID();
    int numThreads = 4;
    if (argc > 1) {
        numThreads = atoi(argv[1]);
    }
    EventLoop loop;
    InetAddress listenAddr(defaultListenPort);
    InetAddress backAddr(defaultBackedIP, defaultBackendPort);
    MultiplexServer server(&loop, listenAddr, backAddr, numThreads);
    server.start();
    loop.loop();
}

