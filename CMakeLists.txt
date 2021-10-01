cmake_minimum_required(VERSION 3.16)

project(comm-unit-tests)

set(CMAKE_CXX_STANDARD 11)

# comm library
add_library(
    comm STATIC

    src/Encoder.cpp
    src/Message.cpp
    src/TcpClient.cpp
    src/TcpServer.cpp
    src/UdpPeer.cpp
)

target_include_directories(
    comm

    PUBLIC
    include
)

target_link_libraries(comm pthread)

# UdpPeer
add_executable(
    peer

    test/ut_peer.cpp
    test/test_vectors.cpp
)

target_include_directories(peer PRIVATE test)

target_link_libraries(peer comm)

# Tcp Client
add_executable(
    tcp_client

    test/ut_tcp_client.cpp
    test/test_vectors.cpp
)

target_include_directories(tcp_client PRIVATE test)

target_link_libraries(tcp_client comm)

# TCP Server
add_executable(
    tcp_server

    test/ut_tcp_server.cpp
    test/test_vectors.cpp
)

target_include_directories(tcp_server PRIVATE test)

target_link_libraries(tcp_server comm)
