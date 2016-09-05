
#include <arale/net/Buffer.h>
#include <arale/net/SocketsOps.h>

#include <errno.h>
#include <sys/uio.h>

namespace arale {

namespace net {

const char Buffer::kCRLF[] = "\r\n";

ssize_t Buffer::readFd(int fd, int *saveErrno) {
    
    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65535];
    struct iovec vec[2];
    size_t writable = writableBytes();
    // data will be written to buffer first
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    ssize_t n = sockets::readv(fd, vec, 2);
    if (n < 0) {
        *saveErrno = errno;
    } else if (implicit_cast<size_t>(n) <= writable) {
        hasWritten(n);
    } else {
        // some data is in extrabuf
        writeIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

}

}
