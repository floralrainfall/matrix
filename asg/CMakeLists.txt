cmake_minimum_required(VERSION 3.7)

project(asg)

add_executable(asg
    src/asg.cpp
)

target_link_libraries(asg matrix)