cmake_minimum_required(VERSION 3.7)

project(rdm)

add_library(rdm MODULE
    include/rdmnet.hpp
    src/rdmnet.cpp
    src/rdm.cpp
)

target_include_directories(rdm PUBLIC include)
target_link_libraries(rdm matrix matrixworld)
