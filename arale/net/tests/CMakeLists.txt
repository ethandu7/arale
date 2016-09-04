
add_executable(eventloop_test00 EventLoop_Test00.cc)
target_link_libraries(eventloop_test00 arale_net)

add_executable(eventloop_test01 EventLoop_Test01.cc)
target_link_libraries(eventloop_test01 arale_net)

add_executable(timer_test00 Timer_Test00.cc)
target_link_libraries(timer_test00 arale_net)

add_executable(socket_test00 Socket_Test00.cc)
target_link_libraries(socket_test00 arale_net)

add_executable(tcpserver_test00 TcpServer_Test00.cc)
target_link_libraries(tcpserver_test00 arale_net)

