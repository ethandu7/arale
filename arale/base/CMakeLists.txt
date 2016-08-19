set(base_SRCS
Thread.cc
Logging.cc
LogStream.cc
Date.cc
TimeStamp.cc
TimeZone.cc
)

add_library(arale_base ${base_SRCS})
target_link_libraries(arale_base pthread rt)

set_target_properties(arale_base PROPERTIES COMPILE_FLAGS "-std=c++0x")

