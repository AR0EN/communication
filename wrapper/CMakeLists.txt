project(comm_wrapper)

add_library(commw SHARED comm_wrapper.cpp)
target_link_libraries(commw comm)
target_include_directories(commw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
