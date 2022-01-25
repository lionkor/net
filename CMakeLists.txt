cmake_minimum_required(VERSION 3.16)

project(lknet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(lknet STATIC include/lk/net/Socket.h src/Socket.cpp)
if (WIN32)
    target_link_libraries(lknet ws2_32)
endif()

option(LK_NET_BUILD_EXAMPLES "Build example(s)" OFF)

if(LK_NET_BUILD_EXAMPLES OR BUILD_EXAMPLES)
    add_executable(lknet-example
        src/main.cpp
    )
    target_link_libraries(lknet-example lknet)
endif()

include_directories(include)
