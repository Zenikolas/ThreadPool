cmake_minimum_required(VERSION 3.6)
project(test_function)

set(CMAKE_CXX_STANDARD 14)

set(SRC ThreadPoolExample.cpp ThreadPool.cpp)

add_executable(test_function ${SRC})