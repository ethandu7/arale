
#include <arale/base/Logging.h>
#include <arale/net/InetAddress.h>
#include <arale/net/SocketsOps.h>
#include <arale/net/Endian.h>
#include <netdb.h>
#include <strings.h>
#include <netinet/in.h>

// INADDR_ANY use (type)value casting
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

namespace arale {

namespace net {

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
    if (ipv6) {
        bzero(&addr6_, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = sockets::hostToNetwork16(port);
    } else {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
        addr_.sin_port = sockets::hostToNetwork16(port);
    }
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6) {
    if (ipv6) {
        bzero(&addr6_, sizeof(addr6_));
        sockets::fromIpPort(ip.c_str(), port, &addr6_);
    } else {
        bzero(&addr_, sizeof(addr_));
        sockets::fromIpPort(ip.c_str(), port, &addr_);
    }
}

string InetAddress::toIpPort() const {
    char buf[64] = "";
    sockets::toIpPort(buf, sizeof(buf), getSockAddr());
    return buf;
}

// don't forget the const
string InetAddress::toIp() const {
    char buf[64] = "";
    sockets::toIp(buf, sizeof(buf), getSockAddr());
    return buf;
}

uint32_t InetAddress::ipNetEndian() const {
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::toPort() const {
    return sockets::networkToHost16(portNetEndian());
}

static __thread char resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress * out) {
    assert(out != NULL);
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    bzero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, resolveBuffer, sizeof(resolveBuffer), &he, &herrno);
    if (ret == 0 && he != NULL) {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    } else {
        if (ret) {
            LOG_SYSERR << "InetAddress::resolve";
        }
        return false;
    }
}

}

}


