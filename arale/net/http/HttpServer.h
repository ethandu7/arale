
#ifndef ARALE_NET_HTTP_HTTPSERVER_H
#define ARALE_NET_HTTP_HTTPSERVER_H

#include <arale/net/TcpServer.h>

namespace arale {

namespace net {

class HttpRequest;
class HttpResponse;

class HttpServer {
public:
    typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpCallback;

    HttpServer(EventLoop *loop, const std::string &name, const InetAddress& addr, 
        TcpServer::Option option = TcpServer::kNoReusePort);

    HttpServer(const HttpServer &) = delete;
    HttpServer& operator=(const HttpServer &) = delete;

    // force out-line dctor, for unique_ptr members
    ~HttpServer();

    void setHttpCallback(const HttpCallback &cb) {
        httpCallback_ = cb;
    }

    EventLoop *getLoop() {
        return server_.getLoop();
    }

    void setThreadNum(int numThread) {
        server_.setThreadNum(numThread);
    }

    void start();
   
private:
    void onConnection(const TcpConnectionPtr &);
    void onMessage(const TcpConnectionPtr &, Buffer *, Timestamp);
    void onRequest(const TcpConnectionPtr &, const HttpRequest &);
    
    TcpServer server_;
    HttpCallback httpCallback_;
};

}

}

#endif
