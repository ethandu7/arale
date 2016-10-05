
add_executable(asio_chatsever ChatServer.cc)
target_link_libraries(asio_chatsever arale_net)

add_executable(asio_chatclient ChatClient.cc)
target_link_libraries(asio_chatclient arale_net)

add_executable(asio_chatsever_threaded ChatServerThreaded.cc)
target_link_libraries(asio_chatsever_threaded arale_net)

add_executable(asio_chatsever_threaded_efficient ChatServerThreadedEfficient.cc)
target_link_libraries(asio_chatsever_threaded_efficient arale_net)
