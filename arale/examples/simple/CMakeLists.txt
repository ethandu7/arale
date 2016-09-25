
add_executable(simple_discard discard/Discard.cc discard/Main.cc)
target_link_libraries(simple_discard arale_net)

add_executable(simple_daytime daytime/Daytime.cc daytime/Main.cc)
target_link_libraries(simple_daytime arale_net)

add_executable(simple_timeserver time/Time.cc time/Main.cc)
target_link_libraries(simple_timeserver arale_net)

add_executable(simple_timeclient time/TimeClient.cc time/Main_Client.cc)
target_link_libraries(simple_timeclient arale_net)

add_executable(simple_chargen chargen/Chargen.cc chargen/Main.cc)
target_link_libraries(simple_chargen arale_net)