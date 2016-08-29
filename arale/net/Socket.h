
#ifndef ARALE_NET_SOCKET_H
#define ARALE_NET_SOCKET_H

// it's in <netinet/tcp.h>
struct tcp_info;

namespace arale {

namespace net {

class InetAddress;

/// It closes the sockfd when desctructs.
/// It's thread safe, all operations are delegated to OS.
class Socket {
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    {  }
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    ~Socket();

    int getSockfd() const { return sockfd_; }
    bool getTcpInfo(struct tcp_info*) const;
    bool getTcpInfoString(char *buf, int len) const;

    /// abort if address in use
    void bindAddress(const InetAddress&);
    /// abort if address in use
    void listen();

    /// On success, returns a non-negative integer that is
    /// a descriptor for the accepted socket, which has been
    /// set to non-blocking and close-on-exec. *peeraddr is assigned.
    /// On error, -1 is returned, and *peeraddr is untouched.
    int accept(InetAddress*);
    void shutDownWrite();

    // TCP_NODELAY(Nagle's algorithm)
    void setTcpNoDelay(bool on);
    // SO_REUSEADDR
    void setReuseAddr(bool on);
    // SO_REUSEPORT
    void setReusePort(bool on);
    // SO_KEEPALIVE
    void setKeepAlive(bool on);
    
private:
    const int sockfd_;
};

}

}

#endif
