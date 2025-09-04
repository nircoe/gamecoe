cmake_minimum_required(VERSION 3.20)
project(gamecoe LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
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

# Fetching libraries for gamecoe:
# glfw
fetch_glfw()
# glad
generate_glad()
# glm
fetch_glm()
# logcoe
fetch_logcoe()

# learnopengl executable
add_executable(learnopengl src/gamecoe/graphics/shader.cpp main.cpp) # main.cpp will be used for learning learnopengl, later on we will add the gamecoe files.

target_sources(learnopengl PRIVATE ${GAMECOE_GLAD_SOURCES})
target_include_directories(learnopengl 
    PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${GAMECOE_GLAD_OUTPUT_DIR}/include"
    "${GAMECOE_CONFIG_DIR}"
)
target_link_libraries(learnopengl
    PRIVATE
        glfw
        logcoe
        stb
        glm::glm-header-only
)
target_compile_features(learnopengl PRIVATE cxx_std_20)

add_dependencies(learnopengl glad)

copy_directory_to_build_root(learnopengl "assets")

if(MSVC)
    target_compile_options(learnopengl PRIVATE /W4)
else()
    target_compile_options(learnopengl PRIVATE -Wall -Wextra -Wpedantic)
endif()
