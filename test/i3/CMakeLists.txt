cmake_minimum_required(VERSION 3.16)
project(tcp-test)

add_executable(client.bin client.cpp)
target_link_libraries(client.bin pthread)

add_executable(server.bin server.cpp)
target_link_libraries(server.bin pthread)
