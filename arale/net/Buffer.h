
#ifndef ARALE_NET_BUFFER_H
#define ARALE_NET_BUFFER_H

#include <arale/base/Types.h>
#include <arale/base/StringPiece.h>
#include <arale/net/Endian.h>

#include <vector>
#include <algorithm>
#include <assert.h>


namespace arale {

namespace net {

class Buffer {
public:
    static const size_t kCheapPrependSize = 8;
    static const size_t kDefaultInitialSize = 1024;
    
    explicit Buffer(size_t initialSize = kDefaultInitialSize):
        buffer_(kCheapPrependSize + kDefaultInitialSize),
        readIndex_(kCheapPrependSize),
        writeIndex_(kCheapPrependSize) 
    {
        assert(writableBytes() == initialSize);
        assert(readableBytes() == 0);
    }
    
    size_t readableBytes() const { return writeIndex_ - readIndex_; }

    void retrieveAll() {
        readIndex_ = writeIndex_ = kCheapPrependSize;
    }

    void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            readIndex_ += len;
        } else {
            retrieveAll();
        }
    }

    size_t writableBytes() const { return buffer_.size() - writeIndex_; }

    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char* beginWrite() { return begin() + writeIndex_; }

    void hasWritten(size_t len) {
        assert(len <= writableBytes());
        writeIndex_ += len;
    }

    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    void append(const StringPiece &str) {
        append(str.data(), str.size());
    }
    
    void append(const void* data, size_t len) {
        append(static_cast<const char*>(data), len);
    }

    void appendInt8(int8_t x) { append(&x, sizeof(x)); }
    
    void appendInt16(int16_t x) { 
        int16_t be16 = sockets::hostToNetwork16(x);
        append(&be16, sizeof(be16));
    }

    void appendInt32(int32_t x) {
        int32_t be32 = sockets::hostToNetwork32(x);
        append(&be32, sizeof(be32));
    }

    void appendInt64(int64_t x) {
        int64_t be64 = sockets::hostToNetwork64(x);
        append(&be64, sizeof(be64));
    }

    /// Read data directly into buffer.
    ///
    /// It may implement with readv(2)
    /// @return result of read(2), @c errno is saved 
    ssize_t readFd(int fd, int *saveErrno);

private:
    // don't return iterator
    char* begin() { return &*buffer_.begin(); }
    
    void makeSpace(size_t len) {
        if (writableBytes() + kCheapPrependSize >= len) {
            // we got move the data to the front part
            size_t copylen = readableBytes();
            std::copy(begin() + readIndex_,
                        begin() + writeIndex_,
                        begin() + kCheapPrependSize);
            readIndex_ = kCheapPrependSize;
            writeIndex_ = readIndex_ + copylen;
        } else {
            buffer_.resize(writeIndex_ + len);
        }
    }
    
    // by using vector<char> we can store binary data
    // std::string would narrow down the types of date can be handle by Buffer
    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;
    static const char kCRLF[];
};

}

}

#endif
