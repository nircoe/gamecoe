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