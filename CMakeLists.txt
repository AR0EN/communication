cmake_minimum_required(VERSION 3.16)

project(Communication)

set(CMAKE_CXX_STANDARD 11)

# add `-DDEFINE_DEBUG=ON` to enable debug log
OPTION(DEFINE_DEBUG "Enable debug log" OFF) # Disabled by default
IF (DEFINE_DEBUG)
message("* Note: Debug Mode!")
message("")
add_definitions(-DDEBUG)
add_compile_options(-g)
ELSE()
message("* Note: pass `-DDEFINE_DEBUG=ON` to enable debug log!")
message("")
add_compile_options(-O3)
ENDIF (DEFINE_DEBUG)

# add `-DDEFINE_PROFILING=ON` to enable profiling
OPTION(DEFINE_PROFILING "Enable profiling" OFF) # Disabled by default
IF (DEFINE_PROFILING)
message("* Note: Profiling is enabled!")
message("")
add_definitions(-DPROFILING)
ELSE()
message("* Note: pass `-DDEFINE_PROFILING=ON` to enable profiling!")
message("")
ENDIF (DEFINE_PROFILING)

add_compile_options(-Wall)
include_directories(include)

add_library(
    comm STATIC
    src/P2P_Endpoint.cpp
    src/TcpClient.cpp
    src/TcpServer.cpp
    src/UdpPeer.cpp
)

set_target_properties(comm PROPERTIES POSITION_INDEPENDENT_CODE True)

if (WIN32)
target_link_libraries(comm ws2_32)
endif (WIN32)

add_subdirectory(wrapper)

# add `-DBUILD_TESTS=ON` to compile unit tests
OPTION(BUILD_TESTS "Enable unit tests" ON) # Enabled by default

# add `-DUSE_RAW_POINTER=ON` to enable debug log
OPTION(DEFINE_USE_RAW_POINTER "Switch to Raw Pointers" OFF) # Disabled by default

IF (BUILD_TESTS)
message("* Note: Tests will be compiled!")
message("")

IF (DEFINE_USE_RAW_POINTER)
message("* Note: Tests will use Raw Pointers!")
message("")
add_definitions(-DUSE_RAW_POINTER)

ENDIF (DEFINE_USE_RAW_POINTER)

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

# Unit test - TCP Client & Server
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
ELSE()
message("* Note: pass `-DBUILD_TESTS=ON` to compile unit tests!")
message("")
ENDIF (BUILD_TESTS)
