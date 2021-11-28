cmake_minimum_required(VERSION 3.16)

project(comm-unit-tests)

set(CMAKE_CXX_STANDARD 11)

# add `-DDEFINE_DEBUG=ON` to enable debug log
OPTION(DEFINE_DEBUG "Enable debug log" OFF) # Disabled by default
IF (DEFINE_DEBUG)
add_definitions(-DDEBUG)
ENDIF (DEFINE_DEBUG)

add_compile_options(-g -Wall)

# Unit test - Encoder
add_executable(
    encoder_ut

    test/encoder_ut.cpp
    test/test_vectors.cpp
)

target_include_directories(encoder_ut PRIVATE include test)
