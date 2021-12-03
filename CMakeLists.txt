cmake_minimum_required(VERSION 3.16)

project(comm-unit-tests)

set(CMAKE_CXX_STANDARD 11)

# add `-DDEFINE_DEBUG=ON` to enable debug log
OPTION(DEFINE_DEBUG "Enable debug log" OFF) # Disabled by default
IF (DEFINE_DEBUG)
add_definitions(-DDEBUG)
ENDIF (DEFINE_DEBUG)

add_compile_options(-g -Wall)

include_directories(include test)

# Test vector
add_library(
    test_vectors STATIC
    test/test_vectors.cpp
)

# Unit test - Encoder
add_executable(
    ut_encoder
    test/ut_encoder.cpp
)

target_link_libraries(ut_encoder test_vectors pthread)

# Unit test - UdpPeer
add_executable(
    ut_udp_peer
    test/ut_udp_peer.cpp
)

target_link_libraries(ut_udp_peer test_vectors pthread)

# Unit test - Tcp Client & Server
add_executable(
    ut_tcp_client
    test/ut_tcp_client.cpp
)

target_link_libraries(ut_tcp_client test_vectors pthread)

add_executable(
    ut_tcp_server
    test/ut_tcp_server.cpp
)

target_link_libraries(ut_tcp_server test_vectors pthread)
