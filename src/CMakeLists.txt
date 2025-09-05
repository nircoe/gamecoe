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
    PUBLIC    
        gamecoe_headers
        logcoe
        stb # TODO: should be moved to PRIVATE later on!
        glfw # TODO: should be moved to PRIVATE later on!
        # soundcoe
        # datacoe
        # poolcoe
        # testcoe
)

target_compile_features(gamecoe 
    PRIVATE 
        cxx_std_20
)

add_dependencies(gamecoe glad)