cmake_minimum_required(VERSION 2.8)

project(simplepost)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")
set(CMAKE_C_COMPILER gcc)

add_executable(simple simplepost.c)

target_link_libraries(simple microhttpd json-c)