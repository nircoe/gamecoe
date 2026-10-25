add_library(gamecoe_headers INTERFACE)

target_include_directories(gamecoe_headers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${GAMECOE_CONFIG_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(gamecoe_headers INTERFACE cxx_std_23)

target_link_libraries(gamecoe_headers
    INTERFACE
        glm::glm-header-only
        $<$<BOOL:${GAMECOE_USE_LOGCOE}>:logcoe>
        $<$<BOOL:${GAMECOE_USE_SOUNDCOE}>:soundcoe>
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)
