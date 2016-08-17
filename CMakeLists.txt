cmake_minimum_required(VERSION 2.6)

project(arale C CXX)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CXX_FLAGS
-g
-DCHECK_PTHREAD_RETURN_VALUE
-D_FILE_OFFSET_BITS=64
-Wall
-Wextra
-Wconversion
-Wno-unused-parameter
-Wold-style-cast
-Woverloaded-virtual
-Wpointer-arith
-Wshadow
-Wwrite-strings
-march=native
-rdynamic
)
