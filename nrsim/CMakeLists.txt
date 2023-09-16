cmake_minimum_required(VERSION 3.7)

project(nrsim)

add_executable(nrsim
    src/nrsim.cpp
)

target_link_libraries(nrsim matrix matrixworld)