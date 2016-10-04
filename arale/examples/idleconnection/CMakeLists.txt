
add_executable(idleconnecion_echoserver Echo.cc Main.cc)
target_link_libraries(idleconnecion_echoserver arale_net)

add_executable(idleconnecion_echoserver2 SortedList.cc)
target_link_libraries(idleconnecion_echoserver2 arale_net)
