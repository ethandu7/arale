
#include <arale/base/Logging.h>
#include <arale/net/EventLoop.h>
#include <arale/net/TcpServer.h>

#include <memory>
#include <stdio.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

static const int kBufsize = 64 * 1024;
static char  *g_file = NULL;

typedef std::shared_ptr<FILE> FilePtr;

void onHighWaterMark(const TcpConnectionPtr &conn, size_t high) {
    LOG_INFO << "HighWaterMark " << high;
}

void onConnection(const TcpConnectionPtr & conn) {
    LOG_INFO << "Chargen Server - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");
    conn->setHighWaterMarkCallback(onHighWaterMark, kBufsize + 1);
    if (conn->isConnected()) {
        FILE *fp = fopen(g_file, "rb");
        if (fp) {
            FilePtr fptr(fp, fclose);
            conn->setContext(fptr);
            char buf[kBufsize];
            size_t nread = fread(buf, 1, kBufsize, fp);
            conn->send(buf, nread);
        } else {
            conn->shutdown();
            LOG_INFO << "FileServer - no such file";
        }
    }
    // no else branch, cause shared_ptr takes care of everything
}

void onWriteComplete(const TcpConnectionPtr & conn) {
    if (!conn->getContext().empty()) {
        FilePtr fptr = boost::any_cast<FilePtr>(conn->getContext());
        char buf[kBufsize];
        size_t nread = fread(buf, 1, kBufsize, fptr.get());
        if (nread > 0) {
            conn->send(buf, nread);
        } else {
            // no close on fp, shared_ptr will take care of closing
            // when server is destructing
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

