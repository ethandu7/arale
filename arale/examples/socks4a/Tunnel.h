
#ifndef ARALE_EXAMPLES_SOCKS4A_TUNNEL_H
#define ARALE_EXAMPLES_SOCKS4A_TUNNEL_H

#include <arale/base/Logging.h>
#include <arale/net/TcpConnection.h>
#include <arale/net/TcpClient.h>

using namespace std::placeholders;

class Tunnel : public std::enable_shared_from_this<Tunnel> {
public:
    Tunnel(arale::net::EventLoop *loop, const arale::net::InetAddress &serverAddr, const arale::net::TcpConnectionPtr &serverConn)
        : client_(loop, serverConn->getName(), serverAddr),
          serverConnection_(serverConn) {
        LOG_INFO << "Tunnel " << serverConn->remoteAddr().toIpPort()
             << " <-> " << serverAddr.toIpPort();
        client_.setConnectionCallback(std::bind(&Tunnel::onClientConneciont, std::shared_from_this(), _1));
        client_.setMessageCallback(std::bind(&Tunnel::onClientMessage, std::shared_from_this(), _1, _2, _3));
        serverConnection_->setHighWaterMarkCallback(std::bind(&Tunnel::onHighWaterMarkWeak, std::weak<Tunnel>(std::shared_from_this()), _1, _2), 10 * 1024 * 1024);
    }

    ~Tunnel() {
        LOG_INFO << "~Tunnel";
    }

    void connect() {
        client_.connect();
    }

    void disconnect() {
        client_.disconnect();
    }
    
private:
    void teardown() {
        // when you are using bind, the arguments must be bound to values or placeholders
        // if no using of bind, no above restriction
        client_.setConnectionCallback(arale::net::defaultConnectionCallback);
        client_.setMessageCallback(arale::net::defaultMessageCallback);
        if (serverConnection_) {
            serverConnection_->setContext(boost::any());
            serverConnection_->shutdown();
        }
    }
    
    void onClientConneciont(const arale::net::TcpConnectionPtr &conn) {
        LOG_DEBUG << (conn->isConnected() ? "UP" : "DOWN");
        if (conn->isConnected()) {
            conn->setTcpNoDelay(true);
            conn->setHighWaterMarkCallback();
            // 
            if (serverConnection_) {
                // hold the connection to the other side
                serverConnection_->setContext(conn);
                if (serverConnection_->getInputBuffer()->readableBytes() > 0) {
                    conn->send(serverConnection_->getInputBuffer());
                }
            } else {
                teardown();
            }
        } else {
            teardown();
        }
    }

    void onClientMessage(const arale::net::TcpConnectionPtr &conn, arale::net::Buffer *buf, arale::Timestamp receiveTime) {
        LOG_DEBUG << conn->getName() << " " << buf->readableBytes();
        if (serverConnection_) {
            serverConnection_->send(buf);
        } else {
            buf->retrieveAll();
            //abort();
            disconnect();
        }
    }

    // if we got data accumulation, close the connection
    void onHighWaterMark(const arale::net::TcpConnectionPtr &conn, size_t bytesToSent) {
        LOG_INFO << "onHighWaterMark " << conn->getName()
                 << " bytes " << bytesToSent;
        disconnect();
    }

    // if we use shared_ptr here, there will be a shared_ptr point to Tunnel object in serverConnnection_
    // at them same time Tunnel object already has a shared_ptr point to serverConnection_
    // classic circular reference
    static void onHighWaterMarkWeak(const std::weak<Tunnel> &weakTunnel, const arale::net::TcpConnectionPtr &conn, size_t bytesToSent) {
        std::shared_ptr<Tunnel> tunnel = weakTunnel.lock();
        if (tunnel) {
            tunnel->onHighWaterMark(conn, bytesToSent);
        }
    }
    
    arale::net::TcpClient client_;
    arale::net::TcpConnectionPtr serverConnection_;
};

#endif
