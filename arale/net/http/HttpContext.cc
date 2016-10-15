
#include <arale/net/http/HttpContext.h>
#include <arale/net/Buffer.h>

namespace arale {

namespace net {

bool HttpContext::parseRequest(Buffer * buf, Timestamp receiveTime) {
    bool hasMore = true;
    bool ok = true;
    while (hasMore) {
        if (parseState_ == kExpectRequestLine) {
            const char *crlf = buf->findCRLF();
            if (crlf) {
                ok = parseRequestLine(buf->peek(), crlf);
                if (ok) {
                    parseState_ = kExpectHeaders;
                    buf->retrieveUntil(crlf + 2);
                    request_.setReceiveTime(receiveTime);
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        } else if (parseState_ == kExpectHeaders) {
            const char *crlf = buf->findCRLF();
            if (crlf) {
                const char *colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    request_.addHeader(buf->peek(), colon, crlf);
                } else {
                    parseState_ = kGotAll;
                    hasMore = false;
                }
                buf->retrieveUntil(crlf + 2);
            } else {
                hasMore = false;
            }
        } else if (parseState_ == kExpectBody) {
            // more to do
        }
    }
    return ok;
}

bool HttpContext::parseRequestLine(const char *start, const char *end) {
    bool ok = false;
    const char *space = std::find(start, end, ' ');
    if (space != end && request_.setMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char *question = std::find(space, end, '?');
            if (question != end) {
                request_.setPath(start, question);
                request_.setQuery(question, space);
            } else {
                request_.setPath(start, space);
            }
            start = space + 1;
            ok = (end - start == 8) && std::equal(start, end - 1, "HTTP/1.");
            if (ok) {
                if (*(end - 1) == '1') {
                    request_.setVersion(HttpRequest::kHttp11);
                } else if (*(end - 1) == '0'){
                    request_.setVersion(HttpRequest::kHttp10);
                } else {
                    ok = false;
                }
            }
        }
    }
    return ok;
}

}

}
