
add_executable(simple_discard discard/Discard.cc discard/Main.cc)
target_link_libraries(simple_discard arale_net)

add_executable(simple_daytime daytime/Daytime.cc daytime/Main.cc)
target_link_libraries(simple_daytime arale_net)

add_executable(simple_timeserver time/Time.cc time/Main.cc)
target_link_libraries(simple_timeserver arale_net)

add_executable(simple_timeclient time/TimeClient.cc time/Main_Client.cc)
target_link_libraries(simple_timeclient arale_net)

add_executable(simple_chargenserver chargen/Chargen.cc chargen/Main.cc)
target_link_libraries(simple_chargenserver arale_net)

add_executable(simple_chargenclient chargen/ChargenClient.cc)
target_link_libraries(simple_chargenclient arale_net)

add_executable(simple_fileserver filetransfer/Download.cc)
target_link_libraries(simple_fileserver arale_net)

