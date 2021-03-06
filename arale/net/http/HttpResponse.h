
#ifndef ARALE_NET_HTTP_HTTPRESPONSE_H
#define ARALE_NET_HTTP_HTTPRESPONSE_H

#include <map>

namespace arale {

namespace net {

class Buffer;

class HttpResponse {
public:
    enum HttpStatusCode {
        kUnknown,
        k200Ok = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404
    };

    explicit HttpResponse(bool close)
        : statusCode_(kUnknown),
          closeConnection_(close) {

    }

    HttpResponse(const HttpResponse&) = delete;
    HttpResponse& operator=(const HttpResponse&) = delete;

    void setStatusCode(HttpStatusCode code) {
        statusCode_ = code;
    }

    void setStatusMessage(const std::string &message) {
        statusMessage_ = message;
    }

    void setCloseConnection(bool on) {
        closeConnection_ = on;
    }

    bool getCloseConnection() {
        return closeConnection_;
    }

    void setContentType(const std::string &contentType) {
        addHeader("Content-Type", contentType);
    }

    void addHeader(const std::string &header, const std::string &value) {
        headers_[header] = value;
    }

    void setBody(const std::string &body) {
        body_ = body;
    }

    void appendToBuffer(Buffer *buf) const;
    
private:
    HttpStatusCode statusCode_;
    std::string statusMessage_;
    std::map<std::string, std::string> headers_;
    std::string body_;
    bool closeConnection_;
};

}

}

#endif
