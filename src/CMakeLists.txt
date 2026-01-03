file(GLOB_RECURSE SOURCES *.cpp)

add_library(gamecoe ${SOURCES})

target_sources(gamecoe PRIVATE ${GAMECOE_GLAD_SOURCES})

target_include_directories(gamecoe
    PRIVATE
        "${GAMECOE_CONFIG_DIR}"
    PUBLIC
        "${GAMECOE_SOURCE_ROOT_DIR}/external" # TODO: should be moved to PRIVATE later on!
        "${GAMECOE_GLAD_OUTPUT_DIR}/include" # TODO: should be moved to PRIVATE later on!
)

target_link_libraries(gamecoe
    PRIVATE
        glm::glm-header-only
        glfw
        stb
    PUBLIC
        gamecoe_headers
        $<$<BOOL:${GAMECOE_USE_LOGCOE}>:logcoe> # how to link it only if GAMECOE_USE_LOGCOE?
        # soundcoe
        # datacoe
        # poolcoe
        # testcoe
)

target_compile_features(gamecoe 
    PRIVATE 
        cxx_std_20
)

if(MSVC)
    target_compile_options(gamecoe PRIVATE /W4 /WX)
else()
    target_compile_options(gamecoe PRIVATE -Werror -Wall -Wextra -Wpedantic)
endif()

add_dependencies(gamecoe glad)