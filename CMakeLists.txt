cmake_minimum_required(VERSION 3.20)

project(http-epoll)

set_property(GLOBAL PROPERTY USE_FOLERS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_SHARED_LIBS "SHARED?" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(src/server)

if( EXISTS "${PROJECT_BINARY_DIR}/compile_commands.json" )
  execute_process( COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${PROJECT_BINARY_DIR}/compile_commands.json
      ${PROJECT_SOURCE_DIR}/compile_commands.json
  )
endif()
