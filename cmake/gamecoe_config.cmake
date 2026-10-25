# gamecoe CMake Configuration
# Master include file for all gamecoe cmake utilities

include(${CMAKE_CURRENT_LIST_DIR}/glfw.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glad.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glm.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/logcoe.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/testcoe.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/soundcoe.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/utils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/config_header.cmake)

# Future modules can be added here
