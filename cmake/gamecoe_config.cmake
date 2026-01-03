# gamecoe CMake Configuration
# Master include file for all gamecoe cmake utilities

include(${CMAKE_CURRENT_LIST_DIR}/c++20.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glfw.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glad.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glm.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/logcoe.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/utils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/config_header.cmake)

# Future modules can be added here

option(GAMECOE_QUIET_GRAPHICS_WARNINGS "Suppress system graphics warnings (libdecor, Mesa)" ON)
if(GAMECOE_QUIET_GRAPHICS_WARNINGS)
    message(STATUS "[gamecoe] Graphics warnings suppressed")
    message(STATUS "[gamecoe] To show warnings: \"set(GAMECOE_QUIET_GRAPHICS_WARNINGS OFF)\" before fetching gamecoe")
    set(GAMECOE_QUIET_GRAPHICS_WARNINGS 1)
else()
    message(STATUS "[gamecoe] Showing all graphics warnings")
    message(STATUS "[gamecoe] To suppress: \"set(GAMECOE_QUIET_GRAPHICS_WARNINGS ON)\" before fetching gamecoe")
    set(GAMECOE_QUIET_GRAPHICS_WARNINGS 0)
endif()
