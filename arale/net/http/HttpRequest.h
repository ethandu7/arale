
#ifndef ARALE_NET_HTTP_HTTPREQUEST_H
#define ARALE_NET_HTTP_HTTPREQUEST_H

#include <arale/base/Logging.h>
#include <arale/base/Types.h>

#include <map>
#include <assert.h>
#include <stdio.h>

namespace arale {

namespace net {

class HttpRequest {
public:
    enum HttpMethod {
        kInvalid,
        kGet,
        kPost,
        kHead,
        kPut,
        kDelete
    };

    enum HttpVersion {
        kUnknown,
        kHttp10,
        kHttp11
    };

    HttpRequest()
        : version_(kUnknown),
          method_(kInvalid) {

    }

    void addHeader(const char *star, const char *colon, const char *crlf) {
        std::string header(star, colon);
        // normally
        // header: value
        ++colon;
        while (colon != crlf && isspace(*colon)) {
            ++colon;
        }
        std::string value(colon, crlf);
        while (!value.empty() && isspace(value[value.size() - 1])) {
            value.resize(value.size() - 1);
        }
        headers_[header] = value;
    }

    std::string getHeader(const std::string &header) {
        std::string res;
        auto it = headers_.find(header);
        if (it != headers_.end()) {
            res = it->second;
        }
        return res;
    }

    const std::map<std::string, std::string>& getHeaders() {
        return headers_;
    }

    void setReceiveTime(Timestamp receiveTime) {
        receiveTime_ = receiveTime;
    }

    void setVersion(HttpVersion version) {
        version_ = version;
    }

    HttpVersion getVersion() {
        return version_;
    }

    bool setMethod(const char *start, const char *end) {
        std::string method(start, end);
        if (method == "GET") {
            method_ = kGet;
        } else if (method == "POST") {
            method_ = kPost;
        } else if (method == "PUT") {
            method_ = kPut;
        } else if (method == "HEAD") {
            method_ = kHead;
        } else if (method == "DELETE") {
            method_ = kDelete;
        } else {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }

    HttpMethod getMethod() {
        return method_;
    }

    const char* methodToString(HttpMethod method) {
        const char *result = "UNKNOWN";
        switch (method) {
            case kGet:
                result = "GET";
                break;
            case kPost:
                result = "POST";
                break;
            case kPut:
                result = "PUT";
                break;
            case kHead:
                result = "HEAD";
                break;
            case kDelete:
                result = "DELETE";
                break;
            default:
                break;
        }
        return result;
    }

    void setPath(const char *start, const char *end) {
        path_.assign(start, end);
    }

    const std::string& getPath() {
        return path_;
    }

    void setQuery(const char *start, const char *end) {
        query_.assign(start, end);
    }

    const std::string& getQuery() {
        return query_;
    }
    
    void swap(HttpRequest& rhs) {
        std::swap(version_, rhs.version_);
        std::swap(method_, rhs.method_);
        path_.swap(rhs.path_);
        query_.swap(rhs.query_);
        receiveTime_.swap(rhs.receiveTime_);
        headers_.swap(rhs.headers_);
    }
    
private:
    HttpVersion version_;
    HttpMethod method_;
    std::string path_;
    std::string query_;
    Timestamp receiveTime_;
    std::map<std::string, std::string> headers_;
};

}

}

#endif
