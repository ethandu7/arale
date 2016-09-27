
#include <arale/base/Logging.h>
#include <arale/net/EventLoop.h>
#include <arale/net/TcpServer.h>

#include <stdio.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

static const char* g_file = NULL;

std::string readFile(const char *fileName) {
    std::string content;
    FILE *fp = fopen(fileName, "rb");
    if (fp) {
        //  the way to read file from disk
        const int kBufSize = 1024 * 1024;
        char ioBuf[kBufSize];
        setbuffer(fp, ioBuf, kBufSize);
        char buf[kBufSize];
        size_t nread = 0;
        while((nread = fread(buf, 1, kBufSize, fp)) > 0) {
            content.append(buf, nread);
        }
        fclose(fp);
    }
    return content;
}

void onHighWaterMark(const TcpConnectionPtr &conn, size_t high) {
    LOG_INFO << "HighWaterMark " << high;
}

void onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "Chargen Server - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");
    if (conn->isConnected()) {
        LOG_INFO << "FileServer - Sending file " << g_file
                 << " to " << conn->remoteAddr().toIpPort();
        conn->setHighWaterMarkCallback(onHighWaterMark, 64 * 1024);
        std::string fileContent = readFile(g_file);
        conn->send(fileContent);
        conn->shutdown();
        LOG_INFO << "FileServer - done";
    }
}

int main(int argc, char *argv[]) {
    LOG_INFO << "pid = " << getCurrentThreadID();
    if (argc  > 1) {
        g_file = argv[1];
        EventLoop loop;
        InetAddress listenAddr(2016);
        TcpServer server(&loop, "File Server", listenAddr);
        // notice, without bind, no placeholders any more
        server.setConnectionCallback(onConnection);
        server.start();
        loop.loop();
    } else {
        fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
    }
}
