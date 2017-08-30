cmake_minimum_required(VERSION 3.8)
project(cpp_tcp_echo_server)

set(CMAKE_CXX_STANDARD 17)

add_executable(cpp_tcp_echo_server_v1 v1_server.cpp)
add_executable(cpp_tcp_echo_server_v1_client v1_client.cpp)