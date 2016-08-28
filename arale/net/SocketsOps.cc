
#include <arale/net/SocketsOps.h>

#include <arale/base/Logging.h>
#include <arale/base/Types.h>
#include <arale/net/Endian.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

typedef struct sockaddr SA;

#if VALGRIND || defined (NO_ACCEPT4)
void setNonBlockAndCloseOnExec(int sockfd) {
    // non-block
    int flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = fcntl(sockfd, F_SETFL, flags);

    // close-on-exec
    flags = fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    int ret = fcntl(sockfd, F_SETFD, flags);
}
#endif

}

namespace arale {

namespace net {

namespace sockets {
    
void fromIpPort(const char * ip, uint16_t port, struct sockaddr_in6 * addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = port;
    if (inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
        LOG_SYSERR << "sockets::fromIpPortv6";
    }
}

void fromIpPort(const char * ip, uint16_t port, struct sockaddr_in * addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = port;
    if (inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        LOG_SYSERR << "sockets::fromIpPortv4";
    }
}


// type safe sockaddr_inX * to sockaddr *
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6 *addr) {
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockaddr_cast(struct sockaddr_in6 *addr) {
    return static_cast<struct sockaddr*>(implicit_cast<void *>(addr));
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr) {
    return static_cast<const struct sockaddr *>(implicit_cast<const void *>(addr));
}

// oppopsite, sockaddr * to sockaddr_inX *
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr)
{
  return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr)
{
  return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

void toIp(char * buf, size_t size, const struct sockaddr * addr) {
    if (addr->sa_family == AF_INET) {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
        inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    } else {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void toIpPort(char * buf, size_t size, const struct sockaddr * addr) {
    toIp(buf, size, addr);
    size_t end = strlen(buf);
    // addr point to sockaddr_in or sockaddr_in6 doesn't matter
    // the port feild in both struct are the same
    const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
    uint16_t port  = networkToHost16(addr4->sin_port);
    snprintf(buf + end, size - end, ":%u", port);
}


}

}

}
