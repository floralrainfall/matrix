cmake_minimum_required(VERSION 3.7)

find_package(CURL REQUIRED)

project(matrixapp)
project(imgui)

add_library(imgui STATIC
  imgui/imgui.h
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_internal.h
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_demo.cpp
  imgui/imstb_rectpack.h
  imgui/imstb_textedit.h
  imgui/imstb_truetype.h
  imgui/imconfig.h

  imgui/backends/imgui_impl_opengl3.h
  imgui/backends/imgui_impl_opengl3.cpp
  imgui/backends/imgui_impl_sdl2.h
  imgui/backends/imgui_impl_sdl2.cpp
)

find_package(SDL2 REQUIRED)

target_include_directories(imgui PUBLIC imgui imgui/backends ${SDL2_INCLUDE_DIR})

add_executable(matrixapp  
  src/matrixapp.cpp
)

set_target_properties(matrixapp
  PROPERTIES
  OUTPUT_NAME matrix-player
)

target_link_options(matrix PRIVATE -export-dynamic)
target_link_options(matrixapp PUBLIC -export-dynamic)
target_link_libraries(matrixapp PUBLIC matrix imgui)
target_include_directories(matrixapp PUBLIC include)
