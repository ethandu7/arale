
#ifndef ARALE_EXAMPLE_ASIO_CHAT_CODEC_H
#define ARALE_EXAMPLE_ASIO_CHAT_CODEC_H

#include <arale/base/Logging.h>

#include <arale/net/Endian.h>
#include <arale/net/Buffer.h>
#include <arale/net/TcpConnection.h>

#include <functional>

class LengthHeaderCodec {
public:
    typedef std::function<void (const arale::net::TcpConnectionPtr &,
                                const std::string &,
                                arale::Timestamp)>  StringMessageCallback;
    LengthHeaderCodec(const StringMessageCallback &cb)
        : messageCallback_(cb) {

    }
    
    void onMessage(const arale::net::TcpConnectionPtr &conn,
                    arale::net::Buffer *buf,
                    arale::Timestamp recevieTime) {
        // try to read as many messages as we can on each time we are called
        while (buf->readableBytes() >= kHeaderLength) {
            const void *data = buf->peek();
            int32_t be32 = *static_cast<const int32_t *>(data);
            int32_t len = arale::net::sockets::networkToHost32(be32);
            if (len > 65535 || len < 0) {
                LOG_ERROR << "Invalid message length " << len;
                conn->shutdown();
                break;
            } else if (buf->readableBytes() >= len + kHeaderLength) {
                // read one message each iteration
                buf->retrieve(kHeaderLength);
                std::string msg = buf->retrieveAsString(arale::implicit_cast<size_t>(len));
                messageCallback_(conn, msg, recevieTime);
            } else {
                // can not get one complete message
                // just return and try to read when we get more data next time
                break;
            }
        }
    }

    void sendMessage(const arale::net::TcpConnectionPtr &conn, const std::string &message) {
        arale::net::Buffer buf;
        buf.append(message.c_str(), message.size());
        int32_t len = static_cast<int32_t>(message.size());
        int32_t be32 = arale::net::sockets::hostToNetwork32(len);
        buf.prepend(&be32, sizeof(int32_t));
        conn->send(&buf);
    }
    
private:
    StringMessageCallback messageCallback_;
    static const size_t kHeaderLength = sizeof(int32_t);
};

#endif

