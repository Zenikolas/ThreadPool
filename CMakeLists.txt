cmake_minimum_required(VERSION 3.6)
project(test_thread_pool)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC ThreadPoolExample.cpp ThreadPool.cpp)

add_definitions(-Wall -O2 -DNDEBUG)

add_executable(test_thread_pool ${SRC})

target_link_libraries(test_thread_pool pthread)