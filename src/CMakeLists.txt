file(GLOB_RECURSE SOURCES *.cpp)

add_library(gamecoe ${SOURCES})

target_sources(gamecoe PRIVATE ${GAMECOE_GLAD_SOURCES})

target_include_directories(gamecoe
    PRIVATE
        "${GAMECOE_SOURCE_ROOT_DIR}/external"
        "${GAMECOE_GLAD_OUTPUT_DIR}/include"
)

target_link_libraries(gamecoe
    PRIVATE
        glfw
        stb
    PUBLIC
        glm::glm-header-only
        gamecoe_headers
        $<$<BOOL:${GAMECOE_USE_LOGCOE}>:logcoe>
        $<$<BOOL:${GAMECOE_USE_SOUNDCOE}>:soundcoe>
        # datacoe
        # poolcoe
        # testcoe
)

target_compile_definitions(gamecoe
    PRIVATE
        GLFW_INCLUDE_NONE
)

gamecoe_enable_strict_warnings(gamecoe)

if(MINGW)
    target_link_options(gamecoe INTERFACE -static-libgcc -static-libstdc++ -static)
endif()

add_dependencies(gamecoe gamecoe_glad)
