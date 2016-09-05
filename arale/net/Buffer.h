
#ifndef ARALE_NET_BUFFER_H
#define ARALE_NET_BUFFER_H

#include <arale/base/Types.h>
#include <arale/base/StringPiece.h>
#include <arale/net/Endian.h>

#include <vector>
#include <algorithm>
#include <assert.h>
#include <string>

namespace arale {

namespace net {

// this Buffer class is not thread safe
// copyable
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

    //////////////////////////////////////////////////////////////////
    // read interfaces
    //////////////////////////////////////////////////////////////////
    size_t readableBytes() { return writeIndex_ - readIndex_; }
    
    char* peek() { return begin() + readIndex_; }

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

    void retrieveUntil(const char *end) {
        //assert(end <= beginWrite());
        assert(end <= begin() + writeIndex_);
        assert(peek() <= end);
        retrieve(end - peek());
    }

    void retrieveInt8() { retrieve(sizeof(int8_t)); }
    void retrieveInt16() { retrieve(sizeof(int16_t)); }
    void retrieveInt32() { retrieve(sizeof(int32_t)); }
    void retrieveInt64() { retrieve(sizeof(int64_t)); }

    std::string retrieveAsString(size_t len) {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    StringPiece toStringPiece() {
        return StringPiece(peek(), static_cast<int>(readableBytes()));
    }
    
    int8_t peekInt8() {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }

    int16_t peekInt16() {
        int16_t x = 0;
        assert(readableBytes() >= sizeof(x));
        memcpy(&x, peek(), sizeof(x));
        return sockets::networkToHost16(x);
    }

    int32_t peekInt32() {
        int32_t x = 0;
        assert(readableBytes() >= sizeof(x));
        memcpy(&x, peek(), sizeof(x));
        return sockets::networkToHost32(x);
    }

    int64_t peekInt64() {
        int64_t x = 0;
        assert(readableBytes() >= sizeof(x));
        memcpy(&x, peek(), sizeof(x));
        return sockets::networkToHost64(x);
    }

    int8_t readInt8() {
        int8_t res = peekInt8();
        retrieveInt8();
        return res;
    }

    int16_t readInt16() {
        int16_t res = peekInt16();
        retrieveInt16();
        return res;
    }

    int32_t readInt32() {
        int32_t res = peekInt32();
        retrieveInt32();
        return res;
    }

    int64_t readInt64() {
        int64_t res = peekInt64();
        retrieveInt64();
        return res;
    }

    //////////////////////////////////////////////////////////////////
    // write interfaces
    //////////////////////////////////////////////////////////////////

    size_t writableBytes() { return buffer_.size() - writeIndex_; }
    
    char* beginWrite() { return begin() + writeIndex_; }

    void hasWritten(size_t len) {
        assert(len <= writableBytes());
        writeIndex_ += len;
    }

    void unwrite(size_t len) {
        assert(len < readableBytes());
        writeIndex_ += len;
    }
    
    void ensureWritableBytes(size_t len) {
            if (writableBytes() < len) {
                makeSpace(len);
            }
            assert(writableBytes() >= len);
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
