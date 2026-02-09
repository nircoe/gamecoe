cmake_minimum_required(VERSION 3.20)
project(gamecoe LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(GAMECOE_SOURCE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(cmake/gamecoe_config.cmake)
check_compiler_version()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_subdirectory(external)

include(FetchContent)

set(GAMECOE_USE_TESTCOE ON) # TODO: delete
# Fetching gamecoe toolkit libraries:
fetch_logcoe()
fetch_testcoe()
fetch_soundcoe()

# Fetching external libraries for gamecoe:
fetch_glfw()
generate_glad()
fetch_glm()

# Generate gamecoe_config.hpp
generate_gamecoe_config_header()

message(STATUS "[gamecoe] Building gamecoe library...")
add_subdirectory(include)
add_subdirectory(src)

if(GAMECOE_USE_TESTCOE)
    add_subdirectory(tests)
endif()

# gamecoe tester executable
# add_executable(tester main.cpp)

# target_link_libraries(tester
#     PRIVATE
#         gamecoe
# )

# copy_gamecoe_assets_to_build_root(tester)

# if(MSVC)
#     target_compile_options(tester PRIVATE /W4 /WX)
# else()
#     target_compile_options(tester PRIVATE -Werror -Wall -Wextra -Wpedantic)
# endif()
