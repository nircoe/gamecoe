# gamecoe CMake Configuration
# Master include file for all gamecoe cmake utilities

include(${CMAKE_CURRENT_LIST_DIR}/c++20.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glfw.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glad.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glm.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/logcoe.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/utils.cmake)

option(GAMECOE_QUIET_GRAPHICS_WARNINGS "Suppress system graphics warnings (libdecor, Mesa)" ON)
if(GAMECOE_QUIET_GRAPHICS_WARNINGS)
    set(GAMECOE_QUIET_GRAPHICS_WARNINGS 1)
else()
    set(GAMECOE_QUIET_GRAPHICS_WARNINGS 0)
endif()

# Future modules can be added here