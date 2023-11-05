cmake_minimum_required(VERSION 3.7)

if(NOT TARGET matrix)
  project(matrix)
endif()

find_package(SDL2 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Bullet REQUIRED)
find_package(ENet REQUIRED)

add_library(matrix STATIC
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

if(MATRIX_COMPILE_GL)
  project(matrix_gl)
  
  add_library(matrix_gl STATIC
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

if(MATRIX_COMPILE_VK)
  find_package(Vulkan REQUIRED)
  project(matrix_vk)

  add_library(matrix_vk STATIC
    include/hw/mvk.hpp
    src/hw/mvk.cpp
  )
  
  target_link_libraries(matrix_vk PUBLIC matrix ${Vulkan_LIBRARIES})

  target_compile_definitions(matrix_vk PUBLIC
    VK_ENABLED
  )
  
  list(APPEND MATRIX_RENDER_APIS matrix_vk)
endif()

if(MATRIX_COMPILE_SDL)
  project(matrix_sdl)
  
  add_library(matrix_sdl STATIC
    include/hw/msdl.hpp
    src/hw/msdl.cpp
  )

  target_compile_definitions(matrix_sdl PUBLIC
    SDL_ENABLED
  )

  target_link_libraries(matrix_sdl PUBLIC ${MATRIX_RENDER_APIS} matrix SDL2)

  list(APPEND MATRIX_FRONTEND_APIS matrix_sdl)
endif()

message(STATUS "render APIs: ${MATRIX_RENDER_APIS}")
message(STATUS "frontend APIs: ${MATRIX_FRONTEND_APIS}")

target_compile_definitions(matrix PUBLIC
  $<$<CONFIG:Debug>:MATRIX_DEBUG=1>
)

target_compile_definitions(matrix PRIVATE
  CONVAR_ENABLE
)

target_link_libraries(matrix PUBLIC 
  glm
  ${MATRIX_FRONTEND_APIS}
  assimp 
  BulletSoftBody 
  BulletDynamics 
  BulletCollision 
  Bullet3Common 
  Bullet3Geometry
  LinearMath
  ${ENET_LIBRARIES}
  ${IRRKLANG_LIBRARIES})
