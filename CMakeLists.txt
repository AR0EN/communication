cmake_minimum_required(VERSION 3.16)

project(comm-unit-tests)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-Wall)

# UdpPeer
add_executable(
    encoder_ut

    test/encoder_ut.cpp
    test/test_vectors.cpp
)

target_include_directories(encoder_ut PRIVATE include test)
