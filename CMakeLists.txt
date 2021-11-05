cmake_minimum_required(VERSION 3.16)

project(lknet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lknet-example
    src/main.cpp
    include/Socket.h src/Socket.cpp
)

include_directories(include)
