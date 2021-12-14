cmake_minimum_required(VERSION 3.16)

project(comm-unit-tests)

set(CMAKE_CXX_STANDARD 11)

# add `-DDEFINE_DEBUG=ON` to enable debug log
OPTION(DEFINE_DEBUG "Enable debug log" OFF) # Disabled by default
IF (DEFINE_DEBUG)
message("* Note: Debug Mode!")
message("")
add_definitions(-DDEBUG)
add_compile_options(-g)
ELSE()
add_compile_options(-O3)
ENDIF (DEFINE_DEBUG)

add_compile_options(-Wall)
include_directories(include)

add_library(
    comm STATIC
    src/P2P_EndPoint.cpp
    src/TcpClient.cpp
    src/TcpServer.cpp
    src/UdpPeer.cpp
)

# add `-DBUILD_TESTS=ON` to compile unit tests
OPTION(BUILD_TESTS "Enable unit tests" ON) # Disabled by default

# add `-DUSE_RAW_POINTER=ON` to enable debug log
OPTION(USE_RAW_POINTER "Switch to Raw Pointers" OFF) # Disabled by default

IF (BUILD_TESTS)
message("* Note: Tests will be compiled!")
message("")

IF (USE_RAW_POINTER)
message("* Note: Tests will use Raw Pointers!")
message("")
add_definitions(-DUSE_RAW_POINTER)

ENDIF (USE_RAW_POINTER)

include_directories(test)

# Test vector
add_library(
    test-vectors STATIC
    test/test_vectors.cpp
)

# Unit test - Encoder
add_executable(
    ut-encoder
    test/ut_encoder.cpp
)

target_link_libraries(ut-encoder comm test-vectors pthread)

# Unit test - UdpPeer
add_executable(
    ut-udp-peer
    test/ut_udp_peer.cpp
)

target_link_libraries(ut-udp-peer comm test-vectors pthread)

# Unit test - Tcp Client & Server
add_executable(
    ut-tcp-client
    test/ut_tcp_client.cpp
)

target_link_libraries(ut-tcp-client comm test-vectors pthread)

add_executable(
    ut-tcp-server
    test/ut_tcp_server.cpp
)

target_link_libraries(ut-tcp-server comm test-vectors pthread)

ENDIF (BUILD_TESTS)
