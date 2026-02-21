cmake_minimum_required(VERSION 3.28)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Allow projects requiring CMake < 3.5" FORCE)
project(server C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE dev CACHE STRING "Choose build type: dev or prod" FORCE)
endif()
set(AVAILABLE_BUILD_TYPES dev prod)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${AVAILABLE_BUILD_TYPES}")

set(FETCHCONTENT_QUIET FALSE)

include(FetchContent)

set(NLOHMANN_JSON_VERSION 3.11.3)
set(NLOHMANN_JSON_URL "https://github.com/nlohmann/json/releases/download/v${NLOHMANN_JSON_VERSION}/json.hpp")
set(NLOHMANN_JSON_INC "${CMAKE_BINARY_DIR}/nlohmann_json/include")
file(MAKE_DIRECTORY "${NLOHMANN_JSON_INC}/nlohmann")
if(NOT EXISTS "${NLOHMANN_JSON_INC}/nlohmann/json.hpp")
	file(DOWNLOAD "${NLOHMANN_JSON_URL}" "${NLOHMANN_JSON_INC}/nlohmann/json.hpp" TLS_VERIFY ON)
endif()
add_library(nlohmann_json INTERFACE)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
target_include_directories(nlohmann_json INTERFACE "${NLOHMANN_JSON_INC}")

set(JSON_VALIDATOR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	json_schema_validator
	GIT_REPOSITORY https://github.com/pboettch/json-schema-validator.git
	GIT_TAG 2.3.0
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
	UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(json_schema_validator)

set(XXHASH_VERSION v0.8.2)
set(XXHASH_INC "${CMAKE_BINARY_DIR}/xxhash/include")
file(MAKE_DIRECTORY "${XXHASH_INC}")
if(NOT EXISTS "${XXHASH_INC}/xxhash.h")
	file(DOWNLOAD
		"https://raw.githubusercontent.com/Cyan4973/xxHash/${XXHASH_VERSION}/xxhash.h"
		"${XXHASH_INC}/xxhash.h"
		TLS_VERIFY ON)
endif()
add_library(xxhash INTERFACE)

# ---- gRPC via FetchContent ------------------------------------------------
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

# ---- Protobuf code generation ---------------------------------------------
set(PROTO_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../proto/core_game.proto")
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
		-I "${CMAKE_CURRENT_SOURCE_DIR}/../proto"
		"${PROTO_SRC}"
	DEPENDS "${PROTO_SRC}" protobuf::protoc grpc_cpp_plugin
	COMMENT "Generating gRPC/Protobuf C++ sources from core_game.proto"
)

add_library(core_game_proto STATIC
	${PROTO_GEN_CC}
	${GRPC_GEN_CC}
)
target_include_directories(core_game_proto PUBLIC "${PROTO_GEN_DIR}")
target_link_libraries(core_game_proto PUBLIC grpc++ protobuf::libprotobuf)
# Suppress warnings in generated code
target_compile_options(core_game_proto PRIVATE -w)

# ---- Build flags -----------------------------------------------------------
set(CMAKE_CXX_FLAGS_PROD "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_PROD   "-O3 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_PROD "")

set(CMAKE_CXX_FLAGS_DEV "${CMAKE_CXX_FLAGS_PROD} -g -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_C_FLAGS_DEV   "${CMAKE_C_FLAGS_PROD} -g -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS_DEV "${CMAKE_EXE_LINKER_FLAGS_PROD} -fsanitize=address")

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(server ${SERVER_SOURCES})
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

set(INC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/inc)
file(GLOB_RECURSE INC_SUBDIRS LIST_DIRECTORIES true "${INC_ROOT}/*")
set(ALL_INC_DIRS ${INC_ROOT})
foreach(d ${INC_SUBDIRS})
	if(IS_DIRECTORY "${d}")
		list(APPEND ALL_INC_DIRS "${d}")
	endif()
endforeach()
list(REMOVE_DUPLICATES ALL_INC_DIRS)
target_include_directories(server PRIVATE ${ALL_INC_DIRS})

target_include_directories(server PRIVATE
	"${CMAKE_BINARY_DIR}/nlohmann_json/include/nlohmann"
	"${CMAKE_BINARY_DIR}/_deps/json_schema_validator-src/src/nlohmann"
	"${CMAKE_BINARY_DIR}/xxhash/include"
)

target_compile_options(server PRIVATE -Wall -Wextra -Werror)

target_link_libraries(server PRIVATE nlohmann_json nlohmann_json_schema_validator xxhash core_game_proto)
