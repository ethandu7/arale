
#include <arale/base/Logging.h>

#include <arale/net/EventLoop.h>

#include <arale/net/http/HttpServer.h>
#include <arale/net/http/HttpRequest.h>
#include <arale/net/http/HttpResponse.h>
#include <arale/net/http/HttpContext.h>

namespace arale {

namespace net {

void defaultHttpCallback(const HttpRequest&conn, HttpResponse *resp) {
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop *loop, const string &name, const InetAddress &addr, TcpServer::Option option) 
    : server_(loop, name, addr, option),
      httpCallback_(defaultHttpCallback) {
    using namespace std::placeholders;
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

HttpServer::~HttpServer() {

}

void HttpServer::start() {
    LOG_INFO << "HttpServer[" << server_.getName()
             << "] starts listenning on " << server_.getIpPort();
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
        conn->setContext(HttpContext());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
    HttpContext *context = boost::any_cast<HttpContext>(conn->getMutableContext());
    // parseRequest will return true if request line is ok, even hearders are not ok
    // if request line is not ok, the request line will be left in buffer
    // if reqeust line is ok, but headers or body are not ok, the headers or body will be left in buffer
    if (context->parseRequest(buf, receiveTime)) {
        // if we didn't get everything, leave data in buffer and handle them next round
        if (context->isGotAll()) {
            onRequest(conn, context->getRequest());
            context->reset();
        }
    } else {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req) {
    const std::string &connection = req.getHeader("Connection");
    bool close = connection == "close" ||
        (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.getCloseConnection()) {
        conn->shutdown();
    }
}

}

}
