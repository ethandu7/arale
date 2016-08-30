
#include <arale/net/SocketsOps.h>
#include <iostream>

using namespace arale;
using namespace arale::net;

int main() {
    int sock = sockets::createNonblockingOrDie(AF_INET);

    int optval = 1;
    int res = setsockopt(sock, SOL_IP, IP_TRANSPARENT, &optval, static_cast<socklen_t>(sizeof(optval)));
    (void)res;
    optval = 0;
    socklen_t optvallen = static_cast<socklen_t>(sizeof(optval));
    res = getsockopt(sock, SOL_IP, IP_TRANSPARENT, &optval, &optvallen);

    if (optval == 1)
        std::cout<<"getsockopt works, the IP_TRANSPARENT is on"<<std::endl;
    else
        std::cout<<"getsockopt doesn't work"<<std::endl;

    sockets::close(sock);
}
