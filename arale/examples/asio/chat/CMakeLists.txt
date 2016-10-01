
add_executable(asio_chatsever ChatServer.cc)
target_link_libraries(asio_chatsever arale_net)

add_executable(asio_chatclient ChatClient.cc)
target_link_libraries(asio_chatclient arale_net)

