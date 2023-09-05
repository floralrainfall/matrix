cmake_minimum_required(VERSION 3.7)

project(rcr)

add_executable(rcr
    src/rcr.cpp
)

target_link_libraries(rcr matrix)