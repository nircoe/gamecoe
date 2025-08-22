# gamecoe CMake Configuration
# Master include file for all gamecoe cmake utilities

include(${CMAKE_CURRENT_LIST_DIR}/c++20.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glfw.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glad.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glm.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/logcoe.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/utils.cmake)

# Future modules can be added here