
#include <arale/net/EventLoop.h>
#include <arale/net/TcpServer.h>

#include <stdio.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

static const int kBufsize = 64 * 1024;
static char  *g_file = NULL;

void onHighWaterMark(const TcpConnectionPtr &conn, size_t high) {
    LOG_INFO << "HighWaterMark " << high;
}

void onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "File Server - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");

    if (conn->isConnected()) {
        LOG_INFO << "FileServer - Sending file " << g_file
                 << " to " << conn->remoteAddr().toIpPort();
        conn->setHighWaterMarkCallback(onHighWaterMark, kBufsize + 1);
        FILE *fp = fopen(g_file, "rb");
        if (fp) {
            conn->setContext(fp);
            char buf[kBufsize];
            size_t nread = fread(buf, 1, kBufsize, fp);
            conn->send(buf, nread);
        } else {
            conn->shutdown();
            LOG_INFO << "FileServer - no such file";
        }
    } else {
        if (!conn->getContext().empty()) {
            FILE *fp = boost::any_cast<FILE *>(conn->getContext());
            if (fp) {
                fclose(fp);
            }
        }
    }
}

void onWriteComplete(const TcpConnectionPtr &conn) {
    if (!conn->getContext().empty()) {
        FILE *fp = boost::any_cast<FILE *>(conn->getContext());
        char buf[kBufsize];
        size_t nread = fread(buf, 1, kBufsize, fp);
        if (nread > 0) {
            conn->send(buf, nread);
        } else {
            fclose(fp);
            fp = NULL;
            conn->setContext(fp);
            conn->shutdown();
            LOG_INFO << "FileServer - done";
        }
    }
}

int main(int argc, char *argv[]) {
    LOG_INFO << "pid = " << getCurrentThreadID();
    if (argc > 1) {
        g_file = argv[1];
        EventLoop loop;
        InetAddress listenAddr(2016);
        TcpServer server(&loop, "File Server", listenAddr);
        // notice, without bind, no placeholders any more
        server.setConnectionCallback(onConnection);
        server.setWriteCompleteCallback(onWriteComplete);
        server.start();
        loop.loop();
    } else {
        fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
    }
}
