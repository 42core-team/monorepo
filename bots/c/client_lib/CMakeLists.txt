cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Allow projects requiring CMake < 3.5" FORCE)
project(core_client_lib C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build type ────────────────────────────────────────────────────────────────

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Use Release for gRPC/dependencies — ASAN is applied only to core_lib below
set(CMAKE_CXX_FLAGS_DEV "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEV   "-O3 -DNDEBUG")

option(CORE_LIB_ASAN "Enable AddressSanitizer on core_lib" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "dev")
	set(CORE_LIB_ASAN ON)
endif()

# ── gRPC via FetchContent ─────────────────────────────────────────────────────

set(FETCHCONTENT_QUIET FALSE)
include(FetchContent)

set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(utf8_range_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(utf8_range_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_CSHARP_EXT OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_CSHARP_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_NODE_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_PHP_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_PYTHON_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_RUBY_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	grpc
	GIT_REPOSITORY https://github.com/grpc/grpc.git
	GIT_TAG v1.62.1
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
	UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(grpc)

# ── Protobuf code generation ─────────────────────────────────────────────────

set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../proto")
set(PROTO_SRC "${PROTO_DIR}/core_game.proto")
set(PROTO_GEN_DIR "${CMAKE_BINARY_DIR}/proto_gen")
file(MAKE_DIRECTORY "${PROTO_GEN_DIR}")

set(PROTO_GEN_CC  "${PROTO_GEN_DIR}/core_game.pb.cc")
set(PROTO_GEN_H   "${PROTO_GEN_DIR}/core_game.pb.h")
set(GRPC_GEN_CC   "${PROTO_GEN_DIR}/core_game.grpc.pb.cc")
set(GRPC_GEN_H    "${PROTO_GEN_DIR}/core_game.grpc.pb.h")

add_custom_command(
	OUTPUT ${PROTO_GEN_CC} ${PROTO_GEN_H} ${GRPC_GEN_CC} ${GRPC_GEN_H}
	COMMAND $<TARGET_FILE:protobuf::protoc>
		--cpp_out=${PROTO_GEN_DIR}
		--grpc_out=${PROTO_GEN_DIR}
		--plugin=protoc-gen-grpc=$<TARGET_FILE:grpc_cpp_plugin>
		-I "${PROTO_DIR}"
		"${PROTO_SRC}"
	DEPENDS "${PROTO_SRC}" protobuf::protoc grpc_cpp_plugin
	COMMENT "Generating gRPC/Protobuf C++ sources from core_game.proto"
)

add_library(core_game_proto STATIC ${PROTO_GEN_CC} ${GRPC_GEN_CC})
target_include_directories(core_game_proto PUBLIC "${PROTO_GEN_DIR}")
target_link_libraries(core_game_proto PUBLIC grpc++ protobuf::libprotobuf)
target_compile_options(core_game_proto PRIVATE -w)

# ── Client library sources ────────────────────────────────────────────────────

set(C_SOURCES
	src/con_lib.c
	src/public/actions.c
	src/public/get.c
	src/public/debug.c
	src/public/printing.c
	src/internal/utils.c
)

set(CXX_SOURCES
	src/internal/grpc_bridge.cpp
)

add_library(core_lib STATIC ${C_SOURCES} ${CXX_SOURCES})

target_include_directories(core_lib PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)
target_include_directories(core_lib PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src/internal
	${PROTO_GEN_DIR}
)

target_link_libraries(core_lib PRIVATE core_game_proto)
target_compile_options(core_lib PRIVATE -Wall -Wextra -Werror)

if(CORE_LIB_ASAN)
	target_compile_options(core_lib PRIVATE -g -fno-omit-frame-pointer -fsanitize=address)
	target_link_options(core_lib PUBLIC -fsanitize=address)
endif()

# ── Output the library to the source tree root for backward compat ────────────

set_target_properties(core_lib PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	OUTPUT_NAME "core_lib"
)
