
add_executable(simple_discard discard/Discard.cc discard/Main.cc)
target_link_libraries(simple_discard arale_net)

add_executable(simple_daytime daytime/Daytime.cc daytime/Main.cc)
target_link_libraries(simple_daytime arale_net)