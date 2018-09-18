cmake_minimum_required(VERSION 3.10)
project(gfb_find)

set(CMAKE_CXX_STANDARD 14)

add_executable(gfb_find main.cpp threads.h)


target_link_libraries(gfb_find
        pthread
        boost_system
        boost_filesystem
        boost_regex
        ${Boost_LIBRARIES}
        )

FIND_PACKAGE(Boost 1.68 COMPONENTS system filesystem program_options regex REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})