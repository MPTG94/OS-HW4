cmake_minimum_required(VERSION 3.17)
project(hw4)

set(CMAKE_CXX_STANDARD 11)

add_executable(hw4 malloc_2.cpp)
add_executable(hw4-t2 main.cpp smalloc.h malloc_2.cpp)
add_executable(hw4-t31 main.cpp smalloc.h malloc_3.cpp)