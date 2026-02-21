cmake_minimum_required(VERSION 3.20)
project(my_core_bot C)

set(CMAKE_C_STANDARD 11)

# Import the client library (builds core_lib + gRPC + protobuf)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../client_lib ${CMAKE_BINARY_DIR}/client_lib)

file(GLOB BOT_SOURCES CONFIGURE_DEPENDS src/*.c)

add_executable(bot ${BOT_SOURCES})
target_include_directories(bot PRIVATE inc ${CMAKE_CURRENT_SOURCE_DIR}/../../client_lib/inc)
target_link_libraries(bot PRIVATE core_lib core_game_proto)
target_compile_options(bot PRIVATE -Wall -Wextra -Werror)
set_target_properties(bot PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
