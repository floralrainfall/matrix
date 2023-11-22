cmake_minimum_required(VERSION 3.7)

project(matrix)
set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(RUNTIME_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")

option(IRRKLANG_DIRECTORY "The directory where irrKlang is stored.")
if(EXISTS ${IRRKLANG_DIRECTORY})
  set(IRRKLANG_INCLUDE_DIR ${IRRKLANG_DIRECTORY}/include)
  if(UNIX)
    # differentiate type of unix/unixlike, finish this at a later date
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      set(IRRKLANG_LIBRARIES
#	${IRRKLANG_DIRECTORY}/bin/linux-gcc-64/ikpFlac.so
#	${IRRKLANG_DIRECTORY}/bin/linux-gcc-64/ikpMP3.so
	${IRRKLANG_DIRECTORY}/bin/linux-gcc-64/libIrrKlang.so
      )
    else()
      message(FATAL_ERROR "Unsupported unixlike environment")
    endif()
  else()
    mesasge(FATAL_ERROR "Unsupported platform")
  endif()
else()
  message(FATAL_ERROR "IRRKLANG_DIRECTORY does not exist or is not set")
endif()
  
add_subdirectory(matrix)
add_subdirectory(matrixworld)

add_subdirectory(rdm)
add_subdirectory(nrsim)
add_subdirectory(asg)
add_subdirectory(awesops)
add_subdirectory(matrixapp)
