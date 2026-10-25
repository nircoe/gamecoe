cmake_minimum_required(VERSION 3.22)
project(gamecoe LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(GAMECOE_SOURCE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(FetchContent)
include(cmake/gamecoe_config.cmake)

option(GAMECOE_BUILD_TESTS "Build the gamecoe test suite" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_subdirectory(external)

# Fetching gamecoe toolkit libraries:
gamecoe_fetch_logcoe()
gamecoe_fetch_soundcoe()
if(GAMECOE_BUILD_TESTS)
    gamecoe_fetch_testcoe()
endif()

# Fetching external libraries for gamecoe:
gamecoe_fetch_glfw()
gamecoe_generate_glad()
gamecoe_fetch_glm()

# Generate gamecoe_config.hpp
gamecoe_generate_config_header()

message(STATUS "[gamecoe] Building gamecoe library...")
add_subdirectory(include)
add_subdirectory(src)

if(GAMECOE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# gamecoe tester executable
# add_executable(tester main.cpp)

# target_link_libraries(tester
#     PRIVATE
#         gamecoe
# )

# gamecoe_copy_assets_to_build_root(tester)

# gamecoe_enable_strict_warnings(tester)
