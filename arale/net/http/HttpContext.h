
#ifndef ARALE_NET_HTTP_HTTPCONTEXT_H
#define ARALE_NET_HTTP_HTTPCONTEXT_H

#include <arale/net/http/HttpRequest.h>

namespace arale {

namespace net {

class Buffer;

class HttpContext {
public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll
    };

    HttpContext() : parseState_(kExpectRequestLine) {

    }

    // default copy-ctor, dtor and assignment are fine
    
    HttpRequest& getRequest() {
        return request_;
    }

    void reset() {
        parseState_ = kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    bool isGotAll() {
        return parseState_ == kGotAll;
    }

    bool parseRequest(Buffer *buf, Timestamp receiveTime);
    
private:
    bool parseRequestLine(const char* start,  const char *end);
    
    HttpRequest request_;
    HttpRequestParseState parseState_;
};

}

}

#endif
