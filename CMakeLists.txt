cmake_minimum_required(VERSION 2.8)

project(simplepost)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu99")
set(CMAKE_C_COMPILER gcc)

add_executable(my_httpd my_httpd.c)

target_link_libraries(my_httpd microhttpd json-c)

add_executable(curl_poster curl_poster.c)

target_link_libraries(curl_poster curl json-c)

add_executable(pong pong.c)

target_link_libraries(pong microhttpd json-c)

add_executable(ping ping.c)

target_link_libraries(ping curl json-c)
