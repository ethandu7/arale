
add_executable(multiplexer_simple MultiplexerSimple.cc)
target_link_libraries(multiplexer_simple arale_net)

add_executable(multiplexer Multiplexer.cc)
target_link_libraries(multiplexer arale_net)

add_executable(demux Demux.cc)
target_link_libraries(demux arale_net)

