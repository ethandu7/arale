
#include "Tunnel.h"

#include <arale/base/ThreadLocal.h>
#include <arale/net/EventLoop.h>
#include <arale/net/TcpServer.h>

#include <map>
#include <stdio.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

// every thread has its own map
static ThreadLocal<std::map<std::string, TunnelPtr> > t_tunnels;
//
static std::mutex g_mutex;
static std::vector<InetAddress> g_backends;
static size_t g_nextIndex = 0;

void onServerConnection(const TcpConnectionPtr & conn) {
    LOG_INFO << "Balancer - " << conn->remoteAddr().toIpPort() << " -> "
                 << conn->localAddr().toIpPort() << " is "
                 << (conn->isConnected() ? "UP" : "DOWN");

    std::map<std::string, TunnelPtr> &tunnels = t_tunnels.getValue();
    if (conn->isConnected()) {
        assert(tunnels.find(conn->getName()) == tunnels.end());
        conn->setTcpNoDelay(true);
        size_t index = 0;
        {
            std::lock_guard<std::mutex> guard(g_mutex);
            index = g_nextIndex;
            g_nextIndex = (g_nextIndex + 1) % g_backends.size();
        }
        InetAddress &serverAddr = g_backends[index];
        TunnelPtr tunnel = TunnelPtr(new Tunnel(conn->getLoop(), serverAddr, conn));
        tunnel->setup();
        tunnel->connect();
        tunnels[conn->getName()] = tunnel;
    } else {
        assert(tunnels.find(conn->getName()) != tunnels.end());
        tunnels[conn->getName()]->disconnect();
        tunnels.erase(conn->getName());
    }
}

void onServerMessage(const TcpConnectionPtr & conn, Buffer * buf, Timestamp receiveTime) {
    if (!conn->getContext().empty()) {
        const TcpConnectionPtr clientConn = boost::any_cast<const TcpConnectionPtr &>(conn->getContext());
        clientConn->send(buf);
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s backend_ip:port [backend_ip:port]\n", argv[0]);
    } else {
        for (int i = 2; i < argc; ++i) {
            std::string hostport = argv[i];
            size_t colon = hostport.find(':');
            if (colon != std::string::npos) {
                string ip = hostport.substr(0, colon);
                uint16_t port = static_cast<uint16_t>(atoi(hostport.c_str() + colon + 1));
                g_backends.push_back(InetAddress(ip, port));
            } else {
                fprintf(stderr, "invalid backend address %s\n", argv[i]);
                return 1;
            }
        }
    
        InetAddress listenAddr(2016);
        EventLoop loop;
        TcpServer server(&loop, "Balancer", listenAddr);
        server.setConnectionCallback(onServerConnection);
        server.setMessageCallback(onServerMessage);
        server.setThreadNum(4);
        server.start();
        loop.loop();
    }
}