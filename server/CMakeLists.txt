cmake_minimum_required(VERSION 3.20)
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

set(NLOHMANN_JSON_VERSION 3.12.0)
set(NLOHMANN_JSON_URL "https://github.com/nlohmann/json/releases/download/v${NLOHMANN_JSON_VERSION}/json.hpp")
# versioned path, so bumping the version downloads the new header instead of reusing the old one
set(NLOHMANN_JSON_INC "${CMAKE_BINARY_DIR}/nlohmann_json-${NLOHMANN_JSON_VERSION}/include")
file(MAKE_DIRECTORY "${NLOHMANN_JSON_INC}/nlohmann")
if(NOT EXISTS "${NLOHMANN_JSON_INC}/nlohmann/json.hpp")
	file(DOWNLOAD "${NLOHMANN_JSON_URL}" "${NLOHMANN_JSON_INC}/nlohmann/json.hpp" TLS_VERIFY ON
		EXPECTED_HASH SHA256=aaf127c04cb31c406e5b04a63f1ae89369fccde6d8fa7cdda1ed4f32dfc5de63)
endif()
add_library(nlohmann_json INTERFACE)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
target_include_directories(nlohmann_json INTERFACE "${NLOHMANN_JSON_INC}")

set(JSON_VALIDATOR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	json_schema_validator
	GIT_REPOSITORY https://github.com/pboettch/json-schema-validator.git
	GIT_TAG 9abddfa905bb4889cc4ae55f87adcaa71dd3bc23 # 2.4.1
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
	UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(json_schema_validator)

set(XXHASH_VERSION v0.8.4)
set(XXHASH_INC "${CMAKE_BINARY_DIR}/xxhash-${XXHASH_VERSION}/include")
file(MAKE_DIRECTORY "${XXHASH_INC}")
if(NOT EXISTS "${XXHASH_INC}/xxhash.h")
	file(DOWNLOAD
		"https://raw.githubusercontent.com/Cyan4973/xxHash/${XXHASH_VERSION}/xxhash.h"
		"${XXHASH_INC}/xxhash.h"
		TLS_VERIFY ON
		EXPECTED_HASH SHA256=3dc8d161e867a62d3417f7885b55fdaded8b7b497e2f165d05c94f4fe24f2ca4)
endif()
add_library(xxhash INTERFACE)

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
	"${NLOHMANN_JSON_INC}/nlohmann"
	"${CMAKE_BINARY_DIR}/_deps/json_schema_validator-src/src/nlohmann"
	"${XXHASH_INC}"
)

target_compile_options(server PRIVATE -Wall -Wextra -Werror)

target_link_libraries(server PRIVATE nlohmann_json nlohmann_json_schema_validator xxhash)
