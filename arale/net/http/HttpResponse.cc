
#include <arale/net/http/HttpResponse.h>
#include <arale/net/Buffer.h>

#include <stdio.h>

namespace arale {

namespace net {

void HttpResponse::appendToBuffer(Buffer *buf) const {
    char buf2[32];
    snprintf(buf2, sizeof(buf2), "HTTP/1.1 %d", statusCode_);
    buf->append(buf2);
    buf->append(statusMessage_);
    buf->append("\r\n");

    if (closeConnection_) {
        buf->append("Connection: close\r\n");
    } else {
        // %zd as size_t
        snprintf(buf2, sizeof(buf2), "Content-Length: %zd\r\n", body_.size());
        buf->append(buf2);
        buf->append("Connection: Keep-Alive\r\n");
    }

    for (auto it = headers_.begin(); it != headers_.end(); ++it) {
        buf->append(it->first);
        buf->append(": ");
        buf->append(it->second);
        buf->append("\r\n");
    }

    buf->append("\r\n");

    buf->append(body_);
}

}

}
