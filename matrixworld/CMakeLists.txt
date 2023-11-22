cmake_minimum_required(VERSION 3.7)

find_package(CURL REQUIRED)

project(matrixworld)

add_library(matrixworld STATIC
  include/picojson.h
  include/mwweb.hpp
  src/mwweb.cpp
  include/mwuser.hpp
  src/mwuser.cpp
)
  
set_target_properties(matrixworld
  PROPERTIES POSITION_INDEPENDENT_CODE
  ON
)
target_link_libraries(matrixworld PUBLIC matrix ${CURL_LIBRARIES})
target_include_directories(matrixworld PUBLIC include ${CURL_INCLUDE_DIR})
