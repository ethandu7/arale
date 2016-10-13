
#include "Tunnel.h"

#include <arale/net/Endian.h>
#include <arale/net/EventLoop.h>
#include <arale/net/TcpServer.h>

#include <map>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

EventLoop *g_loop;
std::map<std::string, TunnelPtr> g_tunnels;

void onServerConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "Socks4a Server - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");

    if (conn->isConnected()) {
        conn->setTcpNoDelay(true);
    } else {
        auto it = g_tunnels.find(conn->getName());
        if (it != g_tunnels.end()) {
            it->second->disconnect();
            g_tunnels.erase(it);
        }
    }
}

void onServerMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
    LOG_INFO << conn->getName() << " " << buf->readableBytes();
    if (g_tunnels.find(conn->getName()) == g_tunnels.end()) {
        // if client doesn't send legal request in 128 bytes, close the client conneciton
        if (buf->readableBytes() > 128) {
            conn->shutdown();
        } else if (buf->readableBytes() > 8) {
            const char* begin = buf->peek() + 8;
            const char* end = buf->peek() + buf->readableBytes();
            const char* where = std::find(begin, end, '\0');
            if (where != end) {
                char ver = buf->peek()[0];
                char cmd = buf->peek()[1];
                const void* port = buf->peek() + 2;
                const void* ip = buf->peek() + 4;
    
                sockaddr_in addr;
                bzero(&addr, sizeof addr);
                addr.sin_family = AF_INET;
                addr.sin_port = *static_cast<const in_port_t *>(port);
                addr.sin_addr.s_addr = *static_cast<const uint32_t *>(ip);
    
                bool socks4a = sockets::networkToHost32(addr.sin_addr.s_addr) < 256;
                bool okay = false;
                if (socks4a) {
                    const char* endOfHostName = std::find(where + 1, end, '\0');
                    if (endOfHostName != end) {
                        string hostname = where + 1;
                        where = endOfHostName;
                        LOG_INFO << "Socks4a host name " << hostname;
                        InetAddress tmp;
                        if (InetAddress::resolve(hostname, &tmp)) {
                            addr.sin_addr.s_addr = tmp.ipNetEndian();
                            okay = true;
                        }
                    } else {
                      return;
                    }
                } else {
                  okay = true;
                }

                InetAddress serverAddr(addr);
                if (ver == 4 && cmd == 1 && okay) {
                  TunnelPtr tunnel(new Tunnel(g_loop, serverAddr, conn));
                  tunnel->setup();
                  tunnel->connect();
                  g_tunnels[conn->getName()] = tunnel;
                  buf->retrieveUntil(where + 1);
                  char response[] = "\000\x5aUVWXYZ";
                  memcpy(response + 2, &addr.sin_port, 2);
                  memcpy(response + 4, &addr.sin_addr.s_addr, 4);
                  conn->send(response, 8);
                } else {
                  char response[] = "\000\x5bUVWXYZ";
                  conn->send(response, 8);
                  conn->shutdown();
                }
            }
        }
    } else if (!conn->getContext().empty()) {
        const TcpConnectionPtr &clientConn = boost::any_cast<const TcpConnectionPtr &>(conn->getContext());
        clientConn->send(buf);
    }
}

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getCurrentThreadID();
    InetAddress listenAddr(2016);
    EventLoop loop;
    g_loop = &loop;
    TcpServer server(&loop, "Socks4a", listenAddr);
    server.setConnectionCallback(onServerConnection);
    server.setMessageCallback(onServerMessage);
    server.start();
    loop.loop();
}