cmake_minimum_required(VERSION 3.7)

project(rdm)

add_executable(rdm
    src/rdm.cpp
)

target_link_libraries(rdm matrix matrixworld)