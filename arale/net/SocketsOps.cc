
#include <arale/base/Logging.h>
#include <arale/base/Types.h>
#include <arale/net/Endian.h>
#include <arale/net/SocketsOps.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

namespace arale {

namespace net {

namespace sockets {
    
void fromIpPort(const char * ip, uint16_t port, struct sockaddr_in6 * addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = hostToNetwork16(port);
    if (inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
        LOG_SYSERR << "sockets::fromIpPortv6";
    }
}

void fromIpPort(const char * ip, uint16_t port, struct sockaddr_in * addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
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



int createNonblockingOrDie(sa_family_t family) {
    int sockfd = socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_SYSFATAL << "[createNonBlockingOrDie]: cannot creat socket";
    }
    return sockfd;
}

void bindOrDie(int sockfd, const struct sockaddr * addr) {
    int ret = bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        LOG_SYSFATAL << "[bindOrDie]: cannot bind address to socket";
    }
}

void listenOrDie(int sockfd) {
    int ret  = listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        LOG_SYSFATAL << "[listenOrDie]: cannot listen on socket";
    }
}

int accept(int sockfd, struct sockaddr_in6 * addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    int connfd = accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (connfd < 0) {
        LOG_SYSFATAL << "[sockets::accept]: cannot accept new connection";
        int savedErrno = errno;
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                LOG_FATAL << "unexcepted error of ::accept" << savedErrno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept" << savedErrno;
                break;
        }
    }
    return connfd;
}

int connect(int sockfd, const struct sockaddr * addr) {
    return connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

ssize_t read(int sockfd, void * buf, size_t count) {
    return ::read(sockfd, buf, count);
}

ssize_t readv(int sockfd, const struct iovec * iov, int iovcnt) {
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t write(int sockfd, const void *buf, size_t count) {
    return ::write(sockfd, buf, count);
}

void close(int sockfd) {
  if (::close(sockfd) < 0)
  {
    LOG_SYSERR << "::close";
  }
}

void shutdownWrite(int sockfd) {
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        LOG_SYSERR << "shutdownWrite";
    }
}

int getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

struct sockaddr_in6 getLocalAddr(int sockfd) {
    struct sockaddr_in6 localaddr;
    bzero(&localaddr, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    if (getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
        LOG_SYSERR << "sockets::getLocalAddr";
    }
    return localaddr;
}

struct sockaddr_in6 getPeerAddr(int sockfd) {
    struct sockaddr_in6 peeraddr;
    bzero(&peeraddr, sizeof(peeraddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));
    if (getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0) {
        LOG_SYSERR << "sockets::getPeerAdddr";
    }
    return peeraddr;
}

bool isSelfConnect(int sockfd) {
    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET) {
        const struct sockaddr_in *laddr = reinterpret_cast<struct sockaddr_in *>(&localaddr);
        const struct sockaddr_in *paddr = reinterpret_cast<struct sockaddr_in *>(&peeraddr);
        return (laddr->sin_port == paddr->sin_port) && 
            (laddr->sin_addr.s_addr == paddr->sin_addr.s_addr);
    } else if(localaddr.sin6_family == AF_INET) {
        return (localaddr.sin6_family == peeraddr.sin6_family) &&
            (memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof(localaddr.sin6_addr)) == 0);
    } else {
        return false;
    }
}

}

}

}
