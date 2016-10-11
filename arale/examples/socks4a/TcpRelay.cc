
#include "Tunnel.h"

#include <arale/net/TcpServer.h>
#include <arale/net/EventLoop.h>

#include <map>
#include <malloc.h>
#include <sys/resource.h>
#include <stdio.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

EventLoop *g_loop;
InetAddress *g_serverAddr;
std::map<std::string, TunnelPtr> g_tunnels;

void onServerConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "TcpRelay - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");

    if (conn->isConnected()) {
        assert(g_tunnels.find(conn->getName()) == g_tunnels.end());
        conn->setTcpNoDelay(true);
        TunnelPtr tunnel(new Tunnel(g_loop, *g_serverAddr, conn));
        tunnel->setup();
        tunnel->connect();
        g_tunnels[conn->getName()] = tunnel;
    } else {
        assert(g_tunnels.find(conn->getName()) != g_tunnels.end());
        g_tunnels[conn->getName()]->disconnect();
        g_tunnels.erase(conn->getName());
    }
}

void onServerMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
    if (!conn->getContext().empty()) {
        const TcpConnectionPtr &clientConn = boost::any_cast<const TcpConnectionPtr &>(conn->getContext());
        clientConn->send(buf);
    }
}

void memstat() {
    malloc_stats();
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <host_ip> <port> <listen_port>\n", argv[0]);
    } else {
        LOG_INFO << "pid = " << getCurrentThreadID();
        // set max virtual memory to 256MB
        {
            size_t kOneMB = 1024 * 1024;
            rlimit rl = { 256 * kOneMB, 256 * kOneMB };
            setrlimit(RLIMIT_AS, &rl);
        }
        const char *ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress serverAddr(ip, port);
        g_serverAddr = &serverAddr;

        uint16_t listenPort = static_cast<uint16_t>(atoi(argv[3]));
        InetAddress listenAddr(listenPort);

        EventLoop loop;
        g_loop = &loop;
        loop.runEvery(3.0, memstat);

        TcpServer server(&loop, "TcpRelay", listenAddr);
        server.setConnectionCallback(onServerConnection);
        server.setMessageCallback(onServerMessage);
        server.start();
        loop.loop();
    }
    
}
