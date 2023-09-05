cmake_minimum_required(VERSION 3.7)

if(NOT TARGET matrix)
    project(matrix)
endif()

project(matrix_gl)

add_library(matrix_gl STATIC
    include/hw/mgl.hpp
    src/hw/mgl.cpp
)

project(matrix_sdl)

add_library(matrix_sdl STATIC
    include/hw/msdl.hpp
    src/hw/msdl.cpp
)

find_package(SDL2 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_library(matrix STATIC
    include/mhwabs.hpp
    include/mmodel.hpp
    include/mgui.hpp
    include/mdev.hpp
    include/mapp.hpp
    include/mfile.hpp
    include/mscene.hpp
    include/mmaterial.hpp
    include/mcfg.hpp
    include/mview.hpp
    include/stb_image.h

    src/mhwabs.cpp
    src/mmodel.cpp
    src/mgui.cpp
    src/mdev.cpp
    src/mapp.cpp
    src/mfile.cpp
    src/mscene.cpp
    src/mmaterial.cpp
    src/mcfg.cpp
    src/mview.cpp
)

target_compile_definitions(matrix PUBLIC
    $<$<CONFIG:Debug>:
        MATRIX_DEBUG=1
    >
)

target_link_libraries(matrix_gl PUBLIC matrix glad)
target_link_libraries(matrix_sdl PUBLIC matrix)
target_link_libraries(matrix PUBLIC SDL2 glm matrix_gl matrix_sdl assimp) 
target_include_directories(matrix PUBLIC include)