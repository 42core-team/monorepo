cmake_minimum_required(VERSION 3.20)
project(gridmaster C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../client_lib ${CMAKE_BINARY_DIR}/client_lib)

file(GLOB BOT_SOURCES CONFIGURE_DEPENDS src/*.c)

add_executable(gridmaster ${BOT_SOURCES})
target_include_directories(gridmaster PRIVATE inc ${CMAKE_CURRENT_SOURCE_DIR}/../../client_lib/inc)
target_link_libraries(gridmaster PRIVATE core_lib core_game_proto)
target_compile_options(gridmaster PRIVATE -Wall -Wextra -Werror)
set_target_properties(gridmaster PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
