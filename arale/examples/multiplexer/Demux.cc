
#include <arale/net/Buffer.h>
#include <arale/net/TcpServer.h>
#include <arale/net/TcpClient.h>
#include <arale/net/EventLoop.h>

#include <map>

using namespace arale;
using namespace arale::net;
using namespace arale::base;
using namespace std::placeholders;

//static const int kMaxConns = 10;
static const size_t kMaxPacketLen = 253;
static const size_t kHeaderLen = 3;

static const uint16_t defaultListenPort = 9999;
static const uint16_t defaultSocksPort = 6666;
static const char* defaultSocksIP = "127.0.0.1";

class DemuxServer {
public:
    DemuxServer(EventLoop *loop, const InetAddress &listenAddr, const InetAddress &socksAddr)
        : loop_(loop),
          server_(loop_, "Demux Server", listenAddr),
          socksAddr_(socksAddr) {
        server_.setConnectionCallback(std::bind(&DemuxServer::onServerConnection, this, _1));
        server_.setMessageCallback(std::bind(&DemuxServer::onServerMessage, this, _1, _2, _3));
    }

    void start() {
        server_.start();
    }

private:
    typedef std::shared_ptr<TcpClient> TcpClientPtr;
    struct Entry {
        int id_;
        TcpConnectionPtr connection_;
        TcpClientPtr client_;
        Buffer buf_;
    };

    void onServerConnection(const TcpConnectionPtr &conn) {
        LOG_INFO << "Demux Server Client - " << conn->remoteAddr().toIpPort() << " -> "
                 << conn->localAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");

        if (conn->isConnected()) {
            if (serverConnection_) {
                serverConnection_->shutdown();
            } else {
                serverConnection_ = conn;
                LOG_INFO << "onServerConnection set serverConnection_";
            }
        } else {
            if (serverConnection_ == conn) {
                serverConnection_.reset();
                socksConnections_.clear();
                LOG_INFO << "onServerConnection reset serverConnection_";
            }
        }
    }

    void onServerMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
        while (buf->readableBytes() > kHeaderLen) {
            size_t len = static_cast<size_t>(*buf->peek());
            if (buf->readableBytes() < len) {
                break;
            } else {
                int id = static_cast<uint8_t>(buf->peek()[1]);
                id |= (static_cast<uint8_t>(buf->peek()[2]) << 8);

                if (id != 0) {
                    assert(socksConnections_.find(id) != socksConnections_.end());
                    TcpConnectionPtr& socksConn = socksConnections_[id].connection_;
                    // connection to socks server is still not establised
                    if (socksConn) {
                        assert(socksConnections_[id].buf_.readableBytes() == 0);
                        socksConn->send(buf->peek() + kHeaderLen, len);
                    } else {
                        socksConnections_[id].buf_.append(buf->peek() + kHeaderLen, len);
                    }
                } else {
                    std::string cmd(buf->peek() + kHeaderLen, len);
                    doCommand(cmd);
                }
                buf->retrieve(kHeaderLen + len);
            }
        }
    }

    void doCommand(const std::string &cmd) {
        static const std::string kConn = "CONN ";

        int id = atoi(&cmd[kConn.size()]);
        bool isUp = cmd.find(" IS UP") != std::string::npos;
        if (isUp) {
            assert(socksConnections_.find(id) == socksConnections_.end());
            char connName[64];
            snprintf(connName, sizeof(connName), "SocksClient %d", id);
	        Entry entry;
            entry.id_ = id;
            entry.client_.reset(new TcpClient(loop_, connName, socksAddr_));
            entry.client_->setConnectionCallback(std::bind(&DemuxServer::onSocksConnection, this, id, _1));
            entry.client_->setMessageCallback(std::bind(&DemuxServer::onSocksMessage, this, id, _1, _2, _3));
            socksConnections_[id] = entry;
            entry.client_->connect();
        } else {
            assert(socksConnections_.find(id) != socksConnections_.end());
            TcpConnectionPtr socksConnection = socksConnections_[id].connection_;
            // there is a possibility that client want to close the connection to
            // socks server but the connection is still not established
            if (socksConnection) {
                socksConnection->shutdown();
            }
            socksConnections_.erase(id);
        }
    }

    void onSocksConnection(int connId, const TcpConnectionPtr &conn) {
        assert(socksConnections_.find(connId) != socksConnections_.end());
        if (conn->isConnected()) {
            socksConnections_[connId].connection_ = conn;
            Buffer &buf = socksConnections_[connId].buf_;
            if (buf.readableBytes() > 0) {
                conn->send(&buf);
            }
        } else {
            // maybe server connection is not there any more
            if (serverConnection_) {
                char buf[64];
                int len = snprintf(buf, sizeof(buf), "DISCONNECT %d", connId);
                Buffer buffer;
                buffer.append(buf, len);
                serverConnection_->send(&buffer);
            } else {
                socksConnections_.erase(connId);
            }
        }
    }

    void onSocksMessage(int connId, const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
        assert(socksConnections_.find(connId) != socksConnections_.end());
        while (buf->readableBytes() > kMaxPacketLen) {
            Buffer buf2;
            buf2.append(buf->peek(), kMaxPacketLen);
            buf->retrieve(kMaxPacketLen);
            sendServerPacket(connId, &buf2);
        }
        if (buf->readableBytes() > 0) {
            sendServerPacket(connId, buf);
        }
    }

    void sendServerPacket(int connId, Buffer *buf) {
        size_t len = buf->readableBytes();
        assert(len <= kMaxPacketLen);
        uint8_t header[kHeaderLen] = {
            static_cast<uint8_t>(len),
            static_cast<uint8_t>(connId & 0xFF),
            static_cast<uint8_t>((connId & 0xFF00) >> 8)
        };
        buf->append(header, kHeaderLen);
        if (serverConnection_) {
            serverConnection_->send(buf);
        }
    }

    typedef std::map<int, Entry> SocksConnectionList;

    EventLoop *loop_;
    TcpServer server_;
    TcpConnectionPtr serverConnection_;
    const InetAddress socksAddr_;
    SocksConnectionList socksConnections_;
};

int main(int argc, char *argv[]) {
    LOG_INFO << "pid = " << getCurrentThreadID();
    EventLoop loop;
    InetAddress listenAddr(defaultListenPort);
    InetAddress socksAddr(defaultSocksIP, defaultSocksPort);
    DemuxServer server(&loop, listenAddr, socksAddr);
    server.start();
    loop.loop();
}

