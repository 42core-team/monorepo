cmake_minimum_required(VERSION 3.20)
project(server CXX)

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

FetchContent_Declare(
	nlohmann_json
	GIT_REPOSITORY https://github.com/nlohmann/json.git
	GIT_TAG v3.11.3
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
	UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

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

target_compile_options(server PRIVATE -Wall -Wextra -Werror)

target_link_libraries(server PRIVATE nlohmann_json_schema_validator)
