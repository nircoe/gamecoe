cmake_minimum_required(VERSION 3.14)
project(gamecoe LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

include(FetchContent)
include(cmake/gamecoe_config.cmake)

# Fetching libraries for gamecoe:
# GLFW
fetch_glfw()
# GLAD
generate_glad()
# logcoe
fetch_logcoe()

# learnopengl executable
add_executable(learnopengl src/gamecoe/shader.cpp main.cpp) # main.cpp will be used for learning learnopengl, later on we will add the gamecoe files.

target_sources(learnopengl PRIVATE ${GAMECOE_GLAD_SOURCES})
target_include_directories(learnopengl 
    PRIVATE 
    "include/"
    "${GAMECOE_GLAD_OUTPUT_DIR}/include"
    "${GAMECOE_CONFIG_DIR}"
)
target_link_libraries(learnopengl
    PRIVATE
        glfw
        logcoe
)
add_dependencies(learnopengl gamecoe_generate_glad)

if(MSVC)
    target_compile_options(learnopengl PRIVATE /W4)
else()
    target_compile_options(learnopengl PRIVATE -Wall -Wextra -Wpedantic)
endif()
