cmake_minimum_required(VERSION 3.7)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT TARGET matrix)
  project(matrix)
endif()

find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Bullet REQUIRED)
find_package(ENet REQUIRED)
find_package(SDL2_ttf REQUIRED)

option(SHARED_LIBRARY OFF)

set(MATRIX_LIBRARY_TYPE STATIC)
set(TARGET_LIBRARY_TYPE STATIC)
if(SHARED_LIBRARY)
  set(MATRIX_LIBRARY_TYPE SHARED)
  set(TARGET_LIBRARY_TYPE MODULE)
endif()

add_library(matrix ${MATRIX_LIBRARY_TYPE}
  include/hw/mkeyboard.hpp
  include/mhwabs.hpp
  include/mmodel.hpp
  include/mgui.hpp
  include/mdev.hpp
  include/mapp.hpp
  include/mfile.hpp
  include/mscene.hpp
  include/mmaterial.hpp
  include/mconsole.hpp
  include/mcvar.hpp
  include/mcfg.hpp
  include/mview.hpp
  include/mbsp.hpp
  include/mphysics.hpp
  include/mschedule.hpp
  include/mnet.hpp
  include/stb_image.h
  
  src/mhwabs.cpp
  src/mmodel.cpp
  src/mgui.cpp
  src/mdev.cpp
  src/mapp.cpp
  src/mfile.cpp
  src/mscene.cpp
  src/mmaterial.cpp
  src/mconsole.cpp
  src/mcvar.cpp
  src/mcfg.cpp
  src/mview.cpp
  src/mbsp.cpp
  src/mphysics.cpp
  src/mschedule.cpp
  src/mnet.cpp
)
  
target_include_directories(matrix PUBLIC 
  include 
  ${BULLET_INCLUDE_DIRS}
  ${ENET_INCLUDE_DIRS}
  ${IRRKLANG_INCLUDE_DIR})

option(MATRIX_COMPILE_GL "Enables the OpenGL3 HWAPI" ON)
option(MATRIX_COMPILE_VK "Enables the Vulkan HWAPI" ON)
option(MATRIX_COMPILE_SDL "Enables the SDL2 frontend" ON)

if(TARGET_LIBRARY_TYPE STREQUAL MODULE)
  message(STATUS adding MODULE_COMPILE)
  add_compile_definitions(MODULE_COMPILE)
endif()

if(MATRIX_COMPILE_GL)
  project(matrix_gl)
  
  add_library(matrix_gl
    include/hw/mgl.hpp
    src/hw/mgl.cpp
  )
  
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glad)
  
  target_link_libraries(matrix_gl PUBLIC matrix glad)

  target_compile_definitions(matrix_gl PUBLIC
    GL_ENABLED
  )

  target_include_directories(matrix_gl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

  list(APPEND MATRIX_RENDER_APIS matrix_gl)
endif()

find_package(Vulkan REQUIRED
  glslang
  SPIRV-Tools
  shaderc_combined)

if(MATRIX_COMPILE_VK AND VULKAN_FOUND)
  project(matrix_vk)

  add_library(matrix_vk STATIC
    include/hw/mvk.hpp
    src/hw/mvk.cpp
  )
  
  target_link_libraries(matrix_vk PUBLIC
    matrix
    Vulkan::Vulkan
    Vulkan::glslang
    SPIRV
    SPIRV-Tools-opt
    pthread
    Vulkan::SPIRV-Tools
    Vulkan::shaderc_combined
  )

  target_compile_definitions(matrix_vk PUBLIC
    VK_ENABLED
  )
  
  list(APPEND MATRIX_RENDER_APIS matrix_vk)
endif()

find_package(SDL2 CONFIG)

if(MATRIX_COMPILE_SDL AND SDL2_FOUND)
  project(matrix_sdl)
  
  add_library(matrix_sdl ${TARGET_LIBRARY_TYPE}
    include/hw/msdl.hpp
    src/hw/msdl.cpp
  )

  target_compile_definitions(matrix_sdl PUBLIC
    SDL_ENABLED
  )

  target_link_libraries(matrix_sdl PUBLIC ${MATRIX_RENDER_APIS} matrix SDL2)

  list(APPEND MATRIX_FRONTEND_APIS matrix_sdl)
endif()

if(UNIX)
  option(MATRIX_COMPILE_MOTIF "Enables the Motif frontend" OFF)
  find_package(Motif)
  
  if(MATRIX_COMPILE_MOTIF AND MATRIX_COMPILE_GL AND MOTIF_FOUND)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED COMPONENTS GLX)
    
    project(matrix_motif)

    add_library(matrix_motif ${TARGET_LIBRARY_TYPE}
      include/hw/mmotif.hpp
      src/hw/mmotif.cpp
    )

    target_compile_definitions(matrix_motif PUBLIC
      MOTIF_ENABLED
    )

    target_link_libraries(matrix_motif PUBLIC
      matrix_gl
      matrix
      ${MOTIF_LIBRARIES}
      X11::Xt
      OpenGL::GLX
    )

    list(APPEND MATRIX_FRONTEND_APIS matrix_motif)
  elseif(MATRIX_COMPILE_MOTIF AND MATRIX_COMPILE_GL)
    message(WARNING "matrix_motif will not be compiled because Motif is missing from your system")
  endif()
endif()

message(STATUS "render APIs: ${MATRIX_RENDER_APIS}")
message(STATUS "frontend APIs: ${MATRIX_FRONTEND_APIS}")

target_compile_definitions(matrix PUBLIC
  $<$<CONFIG:Debug>:MATRIX_DEBUG=1>
)

target_compile_definitions(matrix PRIVATE
  CONVAR_ENABLE
)

if(${TARGET_LIBRARY_TYPE} STREQUAL "MODULE")
  message(STATUS "exporting frontends to module mode")
  unset(MATRIX_FRONTEND_APIS)
endif()

target_link_libraries(matrix PUBLIC 
  glm
  ${MATRIX_FRONTEND_APIS}
  assimp 
  BulletSoftBody 
  BulletDynamics 
  BulletCollision 
  Bullet3Common 
  Bullet3Geometry
  SDL2_ttf
  LinearMath
  ${ENET_LIBRARIES}
  ${IRRKLANG_LIBRARIES})
