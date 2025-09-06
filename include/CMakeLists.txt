add_library(gamecoe_headers INTERFACE)

target_include_directories(gamecoe_headers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(gamecoe_headers
    INTERFACE
        glm::glm-header-only
        logcoe
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/gamecoe.hpp
    DESTINATION include   
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gamecoe
    DESTINATION include/gamecoe
    FILES_MATCHING PATTERN "*.hpp"
)