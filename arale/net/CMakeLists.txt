
set(net_SRCS
EventLoop.cc
Poller.cc
TimerQueue.cc
Channel.cc
Poller.cc
PollPoller.cc
DefaultPoller.cc
Timer.cc
SocketsOps.cc
InetAddress.cc
Socket.cc
Acceptor.cc
TcpServer.cc
TcpConnection.cc
Buffer.cc
EventLoopThread.cc
EventLoopThreadPool.cc
Connector.cc
TcpClient.cc
)

add_library(arale_net ${net_SRCS})
target_link_libraries(arale_net arale_base)
set_target_properties(arale_net PROPERTIES COMPILE_FLAGS "-std=c++0x")

add_subdirectory(tests)
