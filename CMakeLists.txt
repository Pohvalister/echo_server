cmake_minimum_required(VERSION 3.7)
project(Proxy_Server)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp echo_server.cpp echo_server.h)
add_executable(Proxy_Server ${SOURCE_FILES})