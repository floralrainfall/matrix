cmake_minimum_required(VERSION 3.7)

project(awesops)

add_executable(awesops
  include/worker.hpp
  include/player.hpp
  include/input.hpp
  include/map.hpp
  include/game.hpp
  src/awesops.cpp
  src/game.cpp
  src/input.cpp
  src/map.cpp
  src/player.cpp
  src/worker.cpp
)

target_include_directories(awesops PUBLIC include)
target_link_libraries(awesops matrix)
