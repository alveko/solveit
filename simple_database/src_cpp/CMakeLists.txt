cmake_minimum_required(VERSION 3.3)
project(src_cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    simple_database.cpp
    simple_database.hpp)

add_executable(src_cpp ${SOURCE_FILES})