
add_executable(tcprelay TcpRelay.cc)
target_link_libraries(tcprelay arale_net)

add_executable(socks4a Socks4a.cc)
target_link_libraries(socks4a arale_net)

