
set(net_SRCS
EventLoop.cc
Poller.cc
TimerQueue.cc
)

add_library(arale_net ${net_SRCS})
target_link_libraries(arale_net arale_base)
set_target_properties(arale_net PROPERTIES COMPILE_FLAGS "-std=c++0x")

add_subdirectory(tests)
